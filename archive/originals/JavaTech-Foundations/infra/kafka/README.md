# Kafka Infrastructure Setup

This folder contains the Docker Compose configuration for running Kafka in development.

## Quick Start

### Start Kafka Broker

```bash
cd infra/kafka
docker compose up -d
```

The broker will be available at `localhost:9092`.

### Verify Broker Health

```bash
docker compose ps
```

Check that the `kafka` service shows "healthy" status (or wait for healthcheck to pass).

Alternatively, check logs:
```bash
docker compose logs -f kafka
```

### Create/List Topics (Optional)

To create the grades topic (auto-creation is enabled, but you can be explicit):

```bash
docker compose exec kafka kafka-topics.sh \
  --bootstrap-server localhost:9092 \
  --create \
  --topic grades \
  --partitions 3 \
  --replication-factor 1 \
  --if-not-exists

docker compose exec kafka kafka-topics.sh \
  --bootstrap-server localhost:9092 \
  --create \
  --topic grades.DLT \
  --partitions 3 \
  --replication-factor 1 \
  --if-not-exists
```

List all topics:
```bash
docker compose exec kafka kafka-topics.sh \
  --bootstrap-server localhost:9092 \
  --list
```

### Open Console Consumer for Monitoring

Monitor the `grades` topic:
```bash
docker compose exec kafka kafka-console-consumer.sh \
  --bootstrap-server localhost:9092 \
  --topic grades \
  --from-beginning
```

Monitor the DLT topic:
```bash
docker compose exec kafka kafka-console-consumer.sh \
  --bootstrap-server localhost:9092 \
  --topic grades.DLT \
  --from-beginning
```

### Stop Kafka

```bash
docker compose down
```

To also remove volumes and persistent data:
```bash
docker compose down -v
```

## Configuration Details

- **Bootstrap Server**: `localhost:9092`
- **Main Topic**: `grades`
- **DLT Topic**: `grades.DLT`
- **Mode**: KRaft (no Zookeeper)
- **Auto-create Topics**: Enabled
- **Partitions**: 3 (for main topic)
- **Replication Factor**: 1 (development)

## Troubleshooting

If the broker doesn't start:
1. Check Docker is running
2. Ensure port 9092 is not in use
3. View logs: `docker compose logs kafka`
4. Remove old volumes: `docker compose down -v` and retry

If topics don't auto-create, manually create them using the commands above.
