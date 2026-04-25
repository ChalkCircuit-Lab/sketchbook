import { PubSub } from '@google-cloud/pubsub';
import type { FastifyInstance } from 'fastify';
import fp from 'fastify-plugin';

const analyticsEventsPlugin = fp(
  async (fastify: FastifyInstance) => {
    const projectId = fastify.config.GOOGLE_CLOUD_PROJECT;
    const topicName = fastify.config.MOVIE_EVENTS_TOPIC;
    const pubsub = projectId ? new PubSub({ projectId }) : new PubSub();
    const topic = pubsub.topic(topicName);

    fastify.decorate('analyticsEvents', {
      publishMovieViewed: async (event) => {
        if (fastify.config.NODE_ENV === 'test') {
          return;
        }

        const payload = {
          eventType: 'movie_viewed',
          source: 'fast-lazy-bee',
          ...event
        };

        try {
          await topic.publishMessage({
            json: payload,
            attributes: {
              eventType: 'movie_viewed',
              movieId: event.movieId
            }
          });
        } catch (error) {
          fastify.log.error({ err: error, movieId: event.movieId }, 'Failed to publish analytics event');
        }
      }
    });
  },
  { name: 'analytics-events', dependencies: ['server-config'] }
);

export default analyticsEventsPlugin;
