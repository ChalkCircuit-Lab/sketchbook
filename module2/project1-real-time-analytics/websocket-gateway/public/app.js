const stateText = document.getElementById('connection-state');
const statusDot = document.getElementById('status-dot');
const clients = document.getElementById('clients');
const lastUpdate = document.getElementById('last-update');
const topMovies = document.getElementById('top-movies');
const recentActivity = document.getElementById('recent-activity');

function setConnectionState(online) {
  statusDot.className = `dot ${online ? 'online' : 'offline'}`;
  stateText.textContent = online ? 'Connected' : 'Disconnected';
}

function renderTopMovies(items) {
  topMovies.innerHTML = '';
  for (const movie of items || []) {
    const item = document.createElement('li');
    item.innerHTML = `<span class="title">${movie.movieTitle || 'Unknown movie'}</span>
      <span class="meta">${movie.viewCount || 0} views | ${movie.movieId}</span>`;
    topMovies.appendChild(item);
  }
}

function renderRecentActivity(items) {
  recentActivity.innerHTML = '';
  for (const activity of items || []) {
    const item = document.createElement('li');
    item.innerHTML = `<span class="title">${activity.movieTitle || 'Unknown movie'}</span>
      <span class="meta">${activity.viewedAt || activity.processedAt || ''}</span>`;
    recentActivity.appendChild(item);
  }
}

function connect() {
  const protocol = window.location.protocol === 'https:' ? 'wss' : 'ws';
  const socket = new WebSocket(`${protocol}://${window.location.host}/ws`);

  socket.addEventListener('open', () => setConnectionState(true));
  socket.addEventListener('close', () => {
    setConnectionState(false);
    setTimeout(connect, 1500);
  });
  socket.addEventListener('message', (event) => {
    const payload = JSON.parse(event.data);
    if (payload.connectedClients !== undefined) {
      clients.textContent = payload.connectedClients;
    }
    if (payload.generatedAt) {
      const generatedAt = new Date(payload.generatedAt);
      const latestView = (payload.recentActivity || [])
        .map((item) => Date.parse(item.viewedAt))
        .filter((value) => !Number.isNaN(value))
        .sort((a, b) => b - a)[0];
      const latencyText = latestView ? ` | latency ${generatedAt.getTime() - latestView} ms` : '';
      lastUpdate.textContent = `${generatedAt.toLocaleTimeString()}${latencyText}`;
    }
    if (payload.topMovies) {
      renderTopMovies(payload.topMovies);
    }
    if (payload.recentActivity) {
      renderRecentActivity(payload.recentActivity);
    }
  });
}

connect();
