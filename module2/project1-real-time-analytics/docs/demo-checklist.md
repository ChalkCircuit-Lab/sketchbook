# Demo checklist

1. Open the dashboard:

```text
https://websocket-gateway-hm25rt6pha-uc.a.run.app
```

2. Generate a movie view event:

```powershell
curl.exe "https://fast-lazy-bee-hm25rt6pha-uc.a.run.app/api/v1/movies/670f5e20c286545ba702aade"
```

3. Watch the dashboard update the top movie and recent activity list.

4. Useful health checks:

```powershell
curl.exe "https://fast-lazy-bee-hm25rt6pha-uc.a.run.app/api/v1/health"
curl.exe "https://websocket-gateway-hm25rt6pha-uc.a.run.app/health"
```

5. Architecture talking points:

- Cloud Run hosts Service A and the WebSocket Gateway.
- Pub/Sub decouples movie access from analytics processing.
- Cloud Function is the FaaS component.
- Firestore stores aggregate analytics and processed message IDs.
- WebSocket provides real-time dashboard updates.
