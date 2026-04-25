'use strict';

const functions = require('@google-cloud/functions-framework');
const { Firestore, FieldValue } = require('@google-cloud/firestore');
const { PubSub } = require('@google-cloud/pubsub');

const firestore = new Firestore();
const pubsub = new PubSub();

const STATS_COLLECTION = process.env.STATS_COLLECTION || 'movieStats';
const PROCESSED_COLLECTION = process.env.PROCESSED_COLLECTION || 'processedMovieViewEvents';
const RECENT_COLLECTION = process.env.RECENT_COLLECTION || 'recentMovieViews';
const UPDATE_TOPIC = process.env.UPDATE_TOPIC || 'analytics-updates';

function decodePubSubMessage(cloudEvent) {
  const message = cloudEvent.data && cloudEvent.data.message;
  if (!message || !message.data) {
    throw new Error('CloudEvent does not contain a Pub/Sub message');
  }

  return JSON.parse(Buffer.from(message.data, 'base64').toString('utf8'));
}

async function readDashboardSnapshot() {
  const topSnapshot = await firestore
    .collection(STATS_COLLECTION)
    .orderBy('viewCount', 'desc')
    .limit(10)
    .get();
  const recentSnapshot = await firestore
    .collection(RECENT_COLLECTION)
    .orderBy('viewedAt', 'desc')
    .limit(15)
    .get();

  return {
    topMovies: topSnapshot.docs.map((doc) => ({ id: doc.id, ...doc.data() })),
    recentActivity: recentSnapshot.docs.map((doc) => ({ id: doc.id, ...doc.data() })),
    generatedAt: new Date().toISOString()
  };
}

async function processMovieView(cloudEvent) {
  const eventId = cloudEvent.id;
  const event = decodePubSubMessage(cloudEvent);
  const now = new Date().toISOString();
  const movieId = event.movieId;

  if (!movieId) {
    throw new Error('movieId is required');
  }

  const processedRef = firestore.collection(PROCESSED_COLLECTION).doc(eventId);
  const statsRef = firestore.collection(STATS_COLLECTION).doc(movieId);
  const recentRef = firestore.collection(RECENT_COLLECTION).doc(eventId);

  let status = 'processed';
  await firestore.runTransaction(async (transaction) => {
    const processed = await transaction.get(processedRef);
    if (processed.exists) {
      status = 'duplicate';
      return;
    }

    transaction.set(processedRef, {
      processedAt: now,
      movieId,
      sourceEvent: event
    });
    transaction.set(
      statsRef,
      {
        movieId,
        movieTitle: event.movieTitle || 'Unknown movie',
        viewCount: FieldValue.increment(1),
        firstViewedAt: now,
        lastViewedAt: event.viewedAt || now,
        updatedAt: now
      },
      { merge: true }
    );
    transaction.set(recentRef, {
      movieId,
      movieTitle: event.movieTitle || 'Unknown movie',
      viewedAt: event.viewedAt || now,
      requestId: event.requestId || null,
      processedAt: now
    });
  });

  if (status === 'duplicate') {
    console.log(JSON.stringify({ msg: 'Duplicate movie view skipped', eventId, movieId }));
    return;
  }

  const snapshot = await readDashboardSnapshot();
  await pubsub.topic(UPDATE_TOPIC).publishMessage({
    json: {
      type: 'analytics_snapshot',
      eventId,
      changedMovieId: movieId,
      ...snapshot
    },
    attributes: {
      eventType: 'analytics_snapshot',
      movieId
    }
  });

  console.log(JSON.stringify({ msg: 'Movie view processed', eventId, movieId }));
}

functions.cloudEvent('processMovieView', processMovieView);
exports.processMovieView = processMovieView;
