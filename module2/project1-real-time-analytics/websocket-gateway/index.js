'use strict';

const http = require('http');
const path = require('path');
const express = require('express');
const { WebSocketServer } = require('ws');
const { PubSub } = require('@google-cloud/pubsub');
const { Firestore } = require('@google-cloud/firestore');

const PORT = process.env.PORT || 8080;
const SUBSCRIPTION_NAME = process.env.UPDATE_SUBSCRIPTION || 'analytics-updates-gateway-sub';
const STATS_COLLECTION = process.env.STATS_COLLECTION || 'movieStats';
const RECENT_COLLECTION = process.env.RECENT_COLLECTION || 'recentMovieViews';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server, path: '/ws' });
const pubsub = new PubSub();
const firestore = new Firestore();

app.use(express.static(path.join(__dirname, 'public')));

app.get('/health', (req, res) => {
  res.json({ status: 'ok', clients: wss.clients.size });
});

async function buildSnapshot() {
  const topSnapshot = await firestore.collection(STATS_COLLECTION).orderBy('viewCount', 'desc').limit(10).get();
  const recentSnapshot = await firestore.collection(RECENT_COLLECTION).orderBy('viewedAt', 'desc').limit(15).get();

  return {
    type: 'analytics_snapshot',
    topMovies: topSnapshot.docs.map((doc) => ({ id: doc.id, ...doc.data() })),
    recentActivity: recentSnapshot.docs.map((doc) => ({ id: doc.id, ...doc.data() })),
    generatedAt: new Date().toISOString(),
    connectedClients: wss.clients.size
  };
}

function broadcast(payload) {
  const message = JSON.stringify({
    ...payload,
    connectedClients: wss.clients.size
  });

  for (const client of wss.clients) {
    if (client.readyState === client.OPEN) {
      client.send(message);
    }
  }
}

wss.on('connection', async (socket) => {
  socket.send(JSON.stringify({ type: 'connected', connectedClients: wss.clients.size }));
  try {
    socket.send(JSON.stringify(await buildSnapshot()));
  } catch (error) {
    socket.send(JSON.stringify({ type: 'error', message: error.message }));
  }
});

function startPubSubConsumer() {
  const subscription = pubsub.subscription(SUBSCRIPTION_NAME);
  subscription.on('message', (message) => {
    try {
      const payload = JSON.parse(message.data.toString('utf8'));
      broadcast(payload);
      message.ack();
    } catch (error) {
      console.error(JSON.stringify({ msg: 'Failed to handle update message', error: error.message }));
      message.nack();
    }
  });
  subscription.on('error', (error) => {
    console.error(JSON.stringify({ msg: 'Pub/Sub subscription error', error: error.message }));
  });
}

server.listen(PORT, () => {
  console.log(`WebSocket gateway listening on ${PORT}`);
  startPubSubConsumer();
});
