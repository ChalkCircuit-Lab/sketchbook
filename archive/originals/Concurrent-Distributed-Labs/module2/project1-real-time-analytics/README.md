# PCD Project 1 - Real-time Analytics Dashboard

Distributed analytics system for Fast Lazy Bee movie views.

## Components

- `fast-lazy-bee` - base REST API deployed on Cloud Run. `GET /api/v1/movies/:movie_id` publishes a `movie_viewed` event to Pub/Sub.
- `event-processor` - Google Cloud Function triggered by `movie-views`; it deduplicates Pub/Sub deliveries and writes aggregate stats to Firestore.
- `websocket-gateway` - Cloud Run service that consumes `analytics-updates` and broadcasts dashboard snapshots over WebSocket.
- `websocket-gateway/public` - minimal dashboard client.

## Cloud services used

- Cloud Run for Service A and the WebSocket Gateway.
- Pub/Sub for asynchronous communication.
- Cloud Functions for the FaaS event processor.
- Firestore for stateful aggregate analytics.
- Artifact Registry and Cloud Build for container builds.

## MongoDB Atlas setup

Cloud Run cannot use a MongoDB instance running only on this laptop. Use MongoDB Atlas for the base API database.

The Atlas CLI is downloaded locally under `.tools/atlas-cli`. If it is missing, download it with:

```powershell
$tools = Join-Path (Resolve-Path .).Path ".tools"
New-Item -ItemType Directory -Force -Path $tools | Out-Null
Invoke-WebRequest -Uri "https://fastdl.mongodb.org/mongocli/mongodb-atlas-cli_1.54.0_windows_x86_64.zip" -OutFile "$tools\atlas.zip"
Expand-Archive "$tools\atlas.zip" -DestinationPath "$tools\atlas-cli" -Force
```

Atlas login must be run in a normal interactive PowerShell window:

```powershell
.\.tools\atlas-cli\bin\atlas.exe auth login
```

Then list Atlas projects and create the free cluster:

```powershell
.\.tools\atlas-cli\bin\atlas.exe projects list
.\scripts\setup-atlas.ps1 -AtlasProjectId "your-atlas-project-id" -DbPassword "choose-a-strong-password"
```

The setup script prints `MONGO_URL=...`; use that value in the deploy command.

## Deploy

Prerequisites: Google Cloud SDK, billing enabled, MongoDB Atlas connection string for `sample_mflix`.

```powershell
cd "C:\Users\Dragos\OneDrive\Studying Software Engineering\MISS1202O2 Concurrent and Distributed Programming\module2\project1-real-time-analytics"
.\scripts\deploy.ps1 -ProjectId "your-gcp-project-id" -MongoUrl "mongodb+srv://user:pass@cluster.mongodb.net/sample_mflix"
```

After deployment, open the printed dashboard URL and generate traffic:

```powershell
$SERVICE_URL = gcloud run services describe fast-lazy-bee --region us-central1 --format='value(status.url)'
curl.exe "$SERVICE_URL/api/v1/movies/573a1390f29313caabcd4135"
```

Current demo deployment:

- Service A: `https://fast-lazy-bee-hm25rt6pha-uc.a.run.app`
- Dashboard: `https://websocket-gateway-hm25rt6pha-uc.a.run.app`
- Demo movie request: `https://fast-lazy-bee-hm25rt6pha-uc.a.run.app/api/v1/movies/670f5e20c286545ba702aade`

The demo Atlas database contains one seeded movie: `670f5e20c286545ba702aade` / `PCD Demo Movie`.

## Architecture

```mermaid
flowchart LR
  Client[REST client] --> A[Fast Lazy Bee / Cloud Run]
  A -->|movie_viewed| T1[Pub/Sub topic: movie-views]
  T1 --> F[Cloud Function: event-processor]
  F --> DB[(Firestore movieStats)]
  F -->|analytics_snapshot| T2[Pub/Sub topic: analytics-updates]
  T2 --> G[WebSocket Gateway / Cloud Run]
  Browser[Dashboard browser] <-->|WebSocket| G
```

## Metrics to collect

- End-to-end latency: timestamp in `viewedAt` until dashboard `generatedAt`.
- Consistency window: time until Firestore exposes the updated `movieStats` document.
- Cloud Function throughput: Pub/Sub delivery count and function execution count.
- WebSocket reconnection behavior: close the dashboard connection or redeploy the gateway and observe automatic reconnect.
