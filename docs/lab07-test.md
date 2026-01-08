# Lab 7 Messaging Test Guide

This document shows how to manually test the messaging pipeline (QuickGrade → Kafka → PrefSchedule) including retries and DLQ.

## 1. Start Kafka
```powershell
cd infra/kafka
docker compose up -d
```

## 2. Start PrefSchedule
```powershell
cd "Lab04/2. Homework/PrefSchedule"
./mvnw spring-boot:run
```
(or `mvn spring-boot:run`)

## 3. Start QuickGrade
```powershell
cd quickgrade
./mvnw spring-boot:run
```

## 4. Send a valid grade
```powershell
curl -X POST http://localhost:8082/api/grades -H "Content-Type: application/json" -d '{"studentCode":"S001","courseCode":"CS101","grade":9.1}'
```
Expected in PrefSchedule logs:
- Received grade log line
- If `CS101` is marked compulsory, a "Stored grade" log line
- Otherwise an "Ignored" log line

## 5. Send an invalid grade to test retries and DLQ
```powershell
curl -X POST http://localhost:8082/api/grades -H "Content-Type: application/json" -d '{"studentCode":"S001","courseCode":"CS101","grade":99.0}'
```
Expected behavior:
- Listener throws `IllegalArgumentException`
- Retries 3 times (~1s backoff)
- Message is published to `grades.DLT`
- DLT listener logs it with exception details

## 6. Observe topics (optional)
```powershell
cd infra/kafka
# Console consumer for main topic
docker compose exec kafka kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic grades --from-beginning
# Console consumer for DLT
docker compose exec kafka kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic grades.DLT --from-beginning
```

## 7. REST Endpoints in PrefSchedule
- List grades: `GET http://localhost:8080/api/grades`
- Filter by student: `GET http://localhost:8080/api/grades?studentCode=S001`
- Filter by course: `GET http://localhost:8080/api/grades?courseCode=CS101`
- CSV upload (PowerShell):
```powershell
$body = @{ file = Get-Item ".\grades.csv" }
Invoke-RestMethod -Uri http://localhost:8080/api/grades/load-csv -Method Post -Form $body
```
CSV format:
```
S001,CS101,8.5
S002,CS102,7.0
```

Notes:
- Ensure courses exist in PrefSchedule DB and set `compulsory` appropriately.
- H2 in-memory DB resets on restart; for persistence, configure a real database.
