# Complete API Reference

## Base URLs
- **PrefSchedule**: `http://localhost:8081`
- **StableMatch**: `http://localhost:8080`

---

## Instructor Course Preferences API

### 1. Create Single Preference

**Endpoint**: `POST /api/instructor-preferences`

**Authorization**: INSTRUCTOR, ADMIN

**Request**:
```json
{
  "optional_course_id": 10,
  "compulsory_course_code": "Math",
  "percentage": 70.0
}
```

**Response** (201 Created):
```json
{
  "optional_course_id": 10,
  "optional_course_code": "ADS",
  "compulsory_course_code": "Math",
  "percentage": 70.0
}
```

**Error Responses**:
- 400 Bad Request: Invalid percentage (< 0 or > 100)
- 400 Bad Request: Course not found
- 400 Bad Request: Preference already exists
- 400 Bad Request: Course must be optional

---

### 2. Create Batch Preferences

**Endpoint**: `POST /api/instructor-preferences/batch`

**Authorization**: INSTRUCTOR, ADMIN

**Description**: Replace all preferences for a course

**Request**:
```json
{
  "optional_course_id": 10,
  "preferences": [
    {
      "compulsory_course_code": "Math",
      "percentage": 70.0
    },
    {
      "compulsory_course_code": "OOP",
      "percentage": 30.0
    }
  ]
}
```

**Response** (201 Created):
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

**Validations**:
- Total percentage should not exceed 100%
- All percentages must be 0-100
- Course must exist and be optional
- Deletes existing preferences before creating new ones

---

### 3. Get Preferences by Course ID

**Endpoint**: `GET /api/instructor-preferences/course/{courseId}`

**Authorization**: STUDENT, INSTRUCTOR, ADMIN

**Path Parameters**:
- `courseId` (Long): The ID of the course

**Response** (200 OK):
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

**Error Responses**:
- 404 Not Found: Course not found
- 200 OK with empty array: No preferences for course

---

### 4. Get Preferences by Course Code

**Endpoint**: `GET /api/instructor-preferences/course-code/{courseCode}`

**Authorization**: STUDENT, INSTRUCTOR, ADMIN

**Path Parameters**:
- `courseCode` (String): The code of the course (e.g., "ADS", "ML")

**Response** (200 OK):
```json
[
  {
    "optional_course_id": 10,
    "optional_course_code": "ADS",
    "compulsory_course_code": "Math",
    "percentage": 70.0
  }
]
```

---

### 5. Update Preference

**Endpoint**: `PUT /api/instructor-preferences/{preferenceId}`

**Authorization**: INSTRUCTOR, ADMIN

**Path Parameters**:
- `preferenceId` (Long): The ID of the preference to update

**Request**:
```json
{
  "optional_course_id": 10,
  "compulsory_course_code": "Math",
  "percentage": 75.0
}
```

**Response** (200 OK):
```json
{
  "optional_course_id": 10,
  "optional_course_code": "ADS",
  "compulsory_course_code": "Math",
  "percentage": 75.0
}
```

**Error Responses**:
- 404 Not Found: Preference not found
- 400 Bad Request: Invalid percentage

---

### 6. Delete Preference

**Endpoint**: `DELETE /api/instructor-preferences/{preferenceId}`

**Authorization**: INSTRUCTOR, ADMIN

**Path Parameters**:
- `preferenceId` (Long): The ID of the preference to delete

**Response** (204 No Content)

**Error Responses**:
- 404 Not Found: Preference not found

---

### 7. Delete All Preferences for Course

**Endpoint**: `DELETE /api/instructor-preferences/course/{courseId}/all`

**Authorization**: INSTRUCTOR, ADMIN

**Path Parameters**:
- `courseId` (Long): The ID of the course

**Response** (204 No Content)

---

## Course Assignment API

### 8. Execute Assignment Workflow

**Endpoint**: `POST /api/assignments/execute-workflow`

**Authorization**: ADMIN

**Query Parameters**:
- `batchSize` (int, default=5): Number of optional courses per batch

**Description**: Executes the complete assignment workflow for all optional courses

**Response** (200 OK):
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
        },
        {
          "student_id": "1234568",
          "course_id": "ML",
          "student_preference_rank": 2,
          "course_preference_rank": 1
        }
      ],
      "unmatched_students": ["1234569"],
      "full_courses": [],
      "execution_time_ms": 250
    },
    "1": {
      "status": "SUCCESS",
      "message": "Random matching completed (fallback algorithm)",
      "assignments": [...],
      "unmatched_students": [...],
      "full_courses": [...],
      "execution_time_ms": 180
    }
  }
}
```

**Error Responses**:
- 400 Bad Request: Invalid batch size
- 500 Internal Server Error: Workflow execution error

---

## StableMatch API

### 9. Solve Matching Problem

**Endpoint**: `POST /api/v1/matching/solve`

**Request**:
```json
{
  "students": [
    {
      "student_id": "S1",
      "preferences": ["C1", "C2", "C3"]
    },
    {
      "student_id": "S2",
      "preferences": ["C2", "C1"]
    }
  ],
  "courses": [
    {
      "course_id": "C1",
      "preferences": ["S1", "S2"]
    },
    {
      "course_id": "C2",
      "preferences": ["S2", "S1", "S3"]
    }
  ],
  "capacity_per_course": 1
}
```

**Response** (200 OK):
```json
{
  "status": "SUCCESS",
  "message": "Stable matching completed successfully",
  "assignments": [
    {
      "student_id": "S1",
      "course_id": "C1",
      "student_preference_rank": 1,
      "course_preference_rank": 1
    },
    {
      "student_id": "S2",
      "course_id": "C2",
      "student_preference_rank": 1,
      "course_preference_rank": 1
    }
  ],
  "unmatched_students": [],
  "full_courses": [],
  "execution_time_ms": 125
}
```

---

### 10. Get All Assignments

**Endpoint**: `GET /api/v1/matching/assignments`

**Response** (200 OK):
```json
[
  {
    "student_id": "S1",
    "course_id": "C1",
    "student_preference_rank": 1,
    "course_preference_rank": 1
  },
  {
    "student_id": "S2",
    "course_id": "C2",
    "student_preference_rank": 1,
    "course_preference_rank": 1
  }
]
```

---

### 11. Get Student Assignment

**Endpoint**: `GET /api/v1/matching/assignments/student/{studentId}`

**Path Parameters**:
- `studentId` (String): The ID of the student

**Response** (200 OK):
```json
{
  "student_id": "S1",
  "course_id": "C1",
  "student_preference_rank": 1,
  "course_preference_rank": 1
}
```

**Error Responses**:
- 404 Not Found: Student not assigned

---

### 12. Get Course Assignments

**Endpoint**: `GET /api/v1/matching/assignments/course/{courseId}`

**Path Parameters**:
- `courseId` (String): The ID of the course

**Response** (200 OK):
```json
[
  {
    "student_id": "S1",
    "course_id": "C1",
    "student_preference_rank": 1,
    "course_preference_rank": 1
  },
  {
    "student_id": "S3",
    "course_id": "C1",
    "student_preference_rank": 2,
    "course_preference_rank": 3
  }
]
```

---

### 13. Get Unmatched Students

**Endpoint**: `GET /api/v1/matching/unmatched-students`

**Response** (200 OK):
```json
[
  "S4",
  "S5",
  "S6"
]
```

---

### 14. Get Full Courses

**Endpoint**: `GET /api/v1/matching/full-courses`

**Response** (200 OK):
```json
[
  "C1",
  "C2",
  "C3"
]
```

---

### 15. Get Result Summary

**Endpoint**: `GET /api/v1/matching/result-summary`

**Response** (200 OK):
```json
{
  "status": "SUCCESS",
  "message": "Stable matching completed successfully",
  "total_assignments": 150,
  "unmatched_count": 5,
  "full_courses_count": 10,
  "execution_time_ms": 250
}
```

**Response** (when no result yet):
```json
{
  "status": "NO_RESULT",
  "message": null,
  "total_assignments": 0,
  "unmatched_count": 0,
  "full_courses_count": 0,
  "execution_time_ms": 0
}
```

---

### 16. Health Check

**Endpoint**: `GET /api/v1/matching/health`

**Response** (200 OK):
```
StableMatch service is running
```

---

### 17. API Info

**Endpoint**: `GET /api/v1/matching/info`

**Response** (200 OK):
```json
{
  "name": "StableMatch API",
  "version": "1.0.0",
  "description": "Stable Matching Algorithm for Student Course Assignment",
  "endpoint": "/v1/matching/solve",
  "method": "POST"
}
```

---

## Authentication

All endpoints (except health and info) require Bearer token in Authorization header:

```
Authorization: Bearer {jwt_token}
```

### Required Roles

| Endpoint | GET | POST | PUT | DELETE |
|----------|-----|------|-----|--------|
| `/instructor-preferences` | STUDENT/INSTRUCTOR/ADMIN | INSTRUCTOR/ADMIN | INSTRUCTOR/ADMIN | INSTRUCTOR/ADMIN |
| `/assignments/execute-workflow` | - | ADMIN | - | - |

---

## Error Response Format

**Format** (400 Bad Request):
```json
(Empty body or error message in response header)
```

**Format** (404 Not Found):
```json
(Empty body)
```

**Format** (500 Internal Server Error):
```json
{
  "status": "ERROR",
  "message": "Internal server error: {error_details}",
  "assignments": null,
  "unmatched_students": null,
  "full_courses": null,
  "execution_time_ms": 0
}
```

---

## Rate Limits

Currently unlimited. Consider implementing:
- 100 requests per minute for bulk operations
- 1000 requests per minute for queries

---

## Pagination

Currently not implemented. All results returned in single response.

Consider implementing for large datasets:
- `?page=0&size=20`
- Response includes `totalElements`, `totalPages`, etc.

---

## Versioning

Current version: **1.0.0**

API endpoints use versioning prefix:
- `/api/v1/` for PrefSchedule endpoints
- `/api/v1/matching/` for StableMatch endpoints

---

## Example Workflows

### Workflow 1: Setup and Execute

```bash
# 1. Create preferences
curl -X POST http://localhost:8081/api/instructor-preferences/batch \
  -H "Authorization: Bearer {token}" \
  -H "Content-Type: application/json" \
  -d '{...}'

# 2. Verify preferences
curl -X GET http://localhost:8081/api/instructor-preferences/course-code/ADS \
  -H "Authorization: Bearer {token}"

# 3. Execute workflow
curl -X POST "http://localhost:8081/api/assignments/execute-workflow?batchSize=5" \
  -H "Authorization: Bearer {token}"

# 4. Get results
curl -X GET http://localhost:8080/api/v1/matching/result-summary
```

### Workflow 2: Query Results

```bash
# 1. Get all assignments
curl -X GET http://localhost:8080/api/v1/matching/assignments

# 2. Get student assignment
curl -X GET http://localhost:8080/api/v1/matching/assignments/student/1234567

# 3. Get course assignments
curl -X GET http://localhost:8080/api/v1/matching/assignments/course/ADS

# 4. Get unmatched
curl -X GET http://localhost:8080/api/v1/matching/unmatched-students
```

---

## Response Status Codes

| Code | Meaning |
|------|---------|
| 200 | OK - Request succeeded |
| 201 | Created - Resource created |
| 204 | No Content - Deletion successful |
| 400 | Bad Request - Invalid input |
| 401 | Unauthorized - Missing/invalid token |
| 403 | Forbidden - Insufficient permissions |
| 404 | Not Found - Resource not found |
| 500 | Internal Server Error - Server error |

---

## OpenAPI/Swagger

Consider adding Swagger documentation:

```xml
<dependency>
  <groupId>org.springdoc</groupId>
  <artifactId>springdoc-openapi-starter-webmvc-ui</artifactId>
  <version>2.0.0</version>
</dependency>
```

Access at: `http://localhost:8081/swagger-ui.html`

---

## cURL Examples

### Create Preference
```bash
curl -X POST http://localhost:8081/api/instructor-preferences \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "optional_course_id": 10,
    "compulsory_course_code": "Math",
    "percentage": 70.0
  }'
```

### Batch Create
```bash
curl -X POST http://localhost:8081/api/instructor-preferences/batch \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{
    "optional_course_id": 10,
    "preferences": [
      {"compulsory_course_code": "Math", "percentage": 70},
      {"compulsory_course_code": "OOP", "percentage": 30}
    ]
  }'
```

### Execute Workflow
```bash
curl -X POST "http://localhost:8081/api/assignments/execute-workflow?batchSize=5" \
  -H "Authorization: Bearer $TOKEN"
```

### Query Assignments
```bash
curl -X GET http://localhost:8080/api/v1/matching/assignments
curl -X GET http://localhost:8080/api/v1/matching/assignments/student/1234567
curl -X GET http://localhost:8080/api/v1/matching/assignments/course/ADS
```

---

## Notes

- All timestamps are in ISO 8601 format
- All IDs are long integers unless specified as string
- Percentages are decimals (0.0-100.0)
- Empty arrays are returned as `[]`, not null
- Database must be initialized before using endpoints
