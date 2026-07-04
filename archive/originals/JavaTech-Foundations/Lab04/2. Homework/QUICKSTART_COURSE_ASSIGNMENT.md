# Quick Start: Course Assignment System

## Prerequisites

1. **StableMatch Service** running on `http://localhost:8080/api`
2. **Kafka** running on `localhost:9092` (for grade events)
3. **Database** (H2 in-memory or PostgreSQL)
4. **Java 21** and **Maven 3.8+**

## Starting the Services

### 1. Start StableMatch Service

```bash
cd StableMatch
mvn clean package
mvn spring-boot:run
```

Expected output:
```
Started StableMatchApplication in X seconds
```

### 2. Start PrefSchedule Service

```bash
cd Lab04/2. Homework/PrefSchedule
mvn clean package
mvn spring-boot:run
```

## Usage Examples

### Step 1: Set Up Instructor Preferences

Define which compulsory courses are important for an optional course:

```bash
# Create preferences for "Advanced Data Science" course
curl -X POST http://localhost:8081/api/instructor-preferences/batch \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_INSTRUCTOR_TOKEN" \
  -d '{
    "optional_course_id": 10,
    "preferences": [
      {"compulsory_course_code": "Math", "percentage": 70},
      {"compulsory_course_code": "OOP", "percentage": 30}
    ]
  }'
```

Response:
```json
[
  {
    "optional_course_id": 10,
    "optional_course_code": "ADS",
    "compulsory_course_code": "Math",
    "percentage": 70.0
  },
  {
    "optional_course_id": 10,
    "optional_course_code": "ADS",
    "compulsory_course_code": "OOP",
    "percentage": 30.0
  }
]
```

### Step 2: Verify Preferences

```bash
# Get all preferences for a course
curl -X GET http://localhost:8081/api/instructor-preferences/course-code/ADS \
  -H "Authorization: Bearer YOUR_TOKEN"
```

### Step 3: Execute Assignment Workflow

Run the complete assignment process for all optional courses:

```bash
# Execute with batch size of 5 courses
curl -X POST "http://localhost:8081/api/assignments/execute-workflow?batchSize=5" \
  -H "Authorization: Bearer YOUR_ADMIN_TOKEN"
```

Response:
```json
{
  "status": "COMPLETED",
  "message": "Assignment workflow executed successfully",
  "totalBatches": 2,
  "successfulBatches": 2,
  "executionTimeMs": 3500,
  "results": {
    "0": {
      "status": "SUCCESS",
      "message": "Stable matching completed successfully",
      "assignments": [
        {
          "student_id": "1234567",
          "course_id": "ADS",
          "student_preference_rank": 1,
          "course_preference_rank": 2
        }
      ],
      "unmatched_students": ["1234568"],
      "full_courses": [],
      "execution_time_ms": 250
    }
  }
}
```

### Step 4: Query Assignment Results

#### Get all assignments from StableMatch:
```bash
curl -X GET http://localhost:8080/api/v1/matching/assignments
```

#### Get assignment for a specific student:
```bash
curl -X GET "http://localhost:8080/api/v1/matching/assignments/student/1234567"
```

#### Get all assignments for a course:
```bash
curl -X GET "http://localhost:8080/api/v1/matching/assignments/course/ADS"
```

#### Get unmatched students:
```bash
curl -X GET http://localhost:8080/api/v1/matching/unmatched-students
```

## Testing Scenarios

### Scenario 1: Normal Flow with Stable Matching

1. StableMatch service is running
2. Execute workflow
3. Expected: Returns stable matching results

### Scenario 2: Fallback to Random Matching

1. Stop StableMatch service
2. Execute workflow
3. Expected: 
   - First attempt fails
   - Retry triggered (up to 3 times)
   - Fallback to random matching after 5 second timeout
   - Returns random matching results with fallback message

### Scenario 3: Batch Processing

1. Configure batch size = 3
2. Have 10 optional courses
3. Execute workflow
4. Expected: Creates 4 batches (3 + 3 + 3 + 1)

## Configuration Adjustments

Edit `src/main/resources/application.yaml`:

```yaml
# Increase timeout to 10 seconds
stable-match:
  timeout: 10000

# Reduce retry attempts to 2
resilience4j:
  retry:
    instances:
      stableMatchRetry:
        max-attempts: 2
```

## Monitoring and Debugging

### Enable Debug Logging

```yaml
logging:
  level:
    uaic.dbxdrgsl.PrefSchedule.service: DEBUG
    uaic.dbxdrgsl.PrefSchedule.controller: DEBUG
```

### View StableMatch Service Status

```bash
curl -X GET http://localhost:8080/api/v1/matching/health
```

### Check Result Summary

```bash
curl -X GET http://localhost:8080/api/v1/matching/result-summary
```

## Common Issues

### Issue: "StableMatch service unavailable"
- **Solution**: Ensure StableMatch service is running on port 8080
- Check: `curl http://localhost:8080/api/v1/matching/health`

### Issue: "Kafka connection timeout"
- **Solution**: Start Kafka on port 9092
- Or disable Kafka in `application.yaml`

### Issue: "Max retry attempts exceeded"
- **Solution**: Increase timeout or max attempts in configuration
- Check network connectivity to StableMatch

### Issue: "Percentage must be between 0 and 100"
- **Solution**: Verify percentages in preference DTOs
- Ensure total percentage across preferences ≤ 100%

## API Endpoints Summary

| Method | Endpoint | Purpose |
|--------|----------|---------|
| POST | `/api/instructor-preferences` | Create single preference |
| POST | `/api/instructor-preferences/batch` | Create batch preferences |
| GET | `/api/instructor-preferences/course/{id}` | Get preferences by course ID |
| GET | `/api/instructor-preferences/course-code/{code}` | Get preferences by course code |
| PUT | `/api/instructor-preferences/{id}` | Update preference |
| DELETE | `/api/instructor-preferences/{id}` | Delete preference |
| DELETE | `/api/instructor-preferences/course/{id}/all` | Delete all for course |
| POST | `/api/assignments/execute-workflow` | Execute assignment workflow |
| GET | `/api/v1/matching/assignments` | Get all assignments (StableMatch) |
| GET | `/api/v1/matching/assignments/student/{id}` | Get student assignment |
| GET | `/api/v1/matching/assignments/course/{id}` | Get course assignments |
| GET | `/api/v1/matching/unmatched-students` | Get unmatched students |
| GET | `/api/v1/matching/result-summary` | Get result summary |

## Next Steps

1. Customize instructor preferences for your courses
2. Run assignment workflow
3. Verify assignments in database
4. Monitor resilience patterns during failures
5. Adjust batch size and timeout based on performance
