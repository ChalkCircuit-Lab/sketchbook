# Lab 7 Messaging - Testing Guide

This guide covers end-to-end testing of the Kafka messaging pipeline between QuickGrade (publisher) and PrefSchedule (consumer).

## Prerequisites

- Docker Desktop running
- Kafka broker on `localhost:9092`
- PrefSchedule on `http://localhost:8080`
- QuickGrade on `http://localhost:8082`

## Starting Services

### 1. Start Kafka
```powershell
cd "C:\Users\Dragos\OneDrive\Coding Java\JavaTech\infra\kafka"
docker compose up -d
docker ps  # Verify kafka-broker is running
```

### 2. Start PrefSchedule (Consumer)
```powershell
cd "C:\Users\Dragos\OneDrive\Coding Java\JavaTech\Lab04\2. Homework\PrefSchedule"
.\mvnw.cmd spring-boot:run
```
Keep this terminal open. PrefSchedule will log consumed messages.

### 3. Start QuickGrade (Publisher)
Open a **new PowerShell window**:
```powershell
Start-Process powershell -ArgumentList '-NoExit','-Command','cd "C:\Users\Dragos\OneDrive\Coding Java\JavaTech\Lab04\2. Homework\QuickGrade"; .\mvnw.cmd spring-boot:run'
```

Verify ports:
```powershell
netstat -ano | findstr ":8080 :8082"
```

## Test Scenarios

### Test 1: Valid Grade Event (Compulsory Course)

**Publish a valid grade for CS101 (compulsory):**
```powershell
$body='{"studentCode":"S001","courseCode":"CS101","grade":9.1}'
Invoke-WebRequest -Uri http://localhost:8082/api/grades -Method Post -ContentType 'application/json' -Body $body
```

**Expected:**
- HTTP 202 Accepted
- PrefSchedule logs: "Received grade event: GradeEvent[studentCode=S001, courseCode=CS101, grade=9.1]"
- PrefSchedule logs: "Course CS101 is compulsory, storing grade"

**Verify stored in database:**
```powershell
Invoke-RestMethod "http://localhost:8080/api/grades?courseCode=CS101" | ConvertTo-Json -Depth 5
```

**Expected JSON:**
```json
{
  "value": [
    {
      "id": 1,
      "studentCode": "S001",
      "courseCode": "CS101",
      "grade": 9.1,
      "receivedAt": "2025-12-15T09:06:48.084464Z"
    }
  ],
  "Count": 1
}
```

---

### Test 2: Invalid Grade (DLQ Flow)

**Publish an invalid grade (out of range 1.0-10.0):**
```powershell
$body='{"studentCode":"S001","courseCode":"CS101","grade":99.0}'
Invoke-WebRequest -Uri http://localhost:8082/api/grades -Method Post -ContentType 'application/json' -Body $body
```

**Expected:**
- HTTP 202 Accepted
- PrefSchedule logs show 3 retry attempts with errors
- Message sent to Dead Letter Topic (DLT) `grades.DLT`
- DLT listener logs exception details
- **Not stored in database**

**Verify not stored:**
```powershell
Invoke-RestMethod "http://localhost:8080/api/grades" | ConvertTo-Json -Depth 5
```
Should still show only valid grades.

---

### Test 3: Elective Course (Filtering)

**Publish a grade for EL201 (elective, compulsory=false):**
```powershell
$body='{"studentCode":"S002","courseCode":"EL201","grade":8.5}'
Invoke-WebRequest -Uri http://localhost:8082/api/grades -Method Post -ContentType 'application/json' -Body $body
```

**Expected:**
- HTTP 202 Accepted
- PrefSchedule logs: "Course EL201 is not compulsory, ignoring grade"
- **Not stored in database**

**Verify not stored:**
```powershell
Invoke-RestMethod "http://localhost:8080/api/grades?courseCode=EL201" | ConvertTo-Json -Depth 5
```

**Expected:**
```json
{
  "value": [],
  "Count": 0
}
```

---

### Test 4: CSV Upload

**Create a sample CSV file:**
```powershell
@'
S001,CS101,8.5
S002,CS102,9.0
S003,EL201,7.0
'@ | Set-Content -Encoding ascii "$PWD\grades.csv"
```

**Upload to PrefSchedule:**
```powershell
Add-Type -AssemblyName System.Net.Http
$client = New-Object System.Net.Http.HttpClient
$content = New-Object System.Net.Http.MultipartFormDataContent
$fileStream = [System.IO.File]::OpenRead("$PWD\grades.csv")
$fileContent = New-Object System.Net.Http.StreamContent($fileStream)
$fileContent.Headers.ContentType = [System.Net.Http.Headers.MediaTypeHeaderValue]::Parse("text/csv")
$content.Add($fileContent, "file", "grades.csv")
$response = $client.PostAsync("http://localhost:8080/api/grades/load-csv", $content).Result
$response.StatusCode
$response.Content.ReadAsStringAsync().Result
$fileStream.Dispose()
```

**Expected Response:**
```json
{
  "totalRows": 3,
  "storedRows": 2,
  "ignoredRows": 1,
  "failedRows": 0,
  "failures": []
}
```

**Explanation:**
- CS101 and CS102 are compulsory → stored (2 rows)
- EL201 is elective → ignored (1 row)

---

### Test 5: Query Filters

**Get all grades:**
```powershell
Invoke-RestMethod "http://localhost:8080/api/grades" | ConvertTo-Json -Depth 5
```

**Filter by course code:**
```powershell
Invoke-RestMethod "http://localhost:8080/api/grades?courseCode=CS101" | ConvertTo-Json -Depth 5
```

**Filter by student code:**
```powershell
Invoke-RestMethod "http://localhost:8080/api/grades?studentCode=S001" | ConvertTo-Json -Depth 5
```

---

## Demo Courses Seeded at Startup

PrefSchedule automatically creates these courses on startup:

| ID | Code  | Title                        | Credits | Compulsory |
|----|-------|------------------------------|---------|------------|
| 11 | CS101 | Intro to Computer Science    | 5       | ✅ true    |
| 12 | CS102 | Data Structures              | 5       | ✅ true    |
| 13 | EL201 | Elective: Creative Coding    | 4       | ❌ false   |

Use these codes for testing compulsory vs. elective filtering.

---

## Troubleshooting

**Port already in use:**
```powershell
# Find process using port 8080 or 8082
netstat -ano | findstr ":8080"

# Kill the process
Stop-Process -Id <PID> -Force
```

**Kafka not running:**
```powershell
cd "C:\Users\Dragos\OneDrive\Coding Java\JavaTech\infra\kafka"
docker compose down
docker compose up -d
```

**Check Kafka logs:**
```powershell
docker logs kafka-broker
```

**QuickGrade keeps stopping in VS Code:**
Use `Start-Process` to launch in a separate window (see step 3 above).

---

## Architecture

```
QuickGrade (8082)
    ↓ POST /api/grades
    ↓ Publish GradeEvent
Kafka Topic: grades
    ↓ Consume
PrefSchedule (8080)
    ↓ Validate (1.0-10.0)
    ↓ Check compulsory flag
    ↓ Store if compulsory
H2 Database (in-memory)
    ↓ Query via
GET /api/grades
```

**DLQ Flow:**
```
Invalid Message → 3 Retries (1s backoff) → grades.DLT → DLT Listener (logs only)
```

---

## Lab 7 Requirements Coverage

- ✅ **Compulsory (1p):** Kafka broker running, QuickGrade publishes events, PrefSchedule consumes and prints to console
- ✅ **Homework (2p):** 
  - Grades stored in database table
  - Only compulsory courses stored (elective filtering)
  - REST endpoints: GET with filters, POST CSV upload
  - DLQ handler with retry semantics (3 retries, 1s backoff)
  - Failed messages routed to `grades.DLT` topic

---

## Clean Up

**Stop all services:**
```powershell
# Stop Spring Boot apps (Ctrl+C in their terminals)

# Stop Kafka
cd "C:\Users\Dragos\OneDrive\Coding Java\JavaTech\infra\kafka"
docker compose down
```
