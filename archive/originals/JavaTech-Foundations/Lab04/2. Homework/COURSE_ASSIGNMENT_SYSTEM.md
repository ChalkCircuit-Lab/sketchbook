# Course Assignment System Implementation

## Overview

This document describes the enhanced course assignment system that integrates instructor course preferences with the StableMatch microservice for optimal student-to-course assignment.

## Components

### 1. Instructor Course Preferences

#### Model: `InstructorCoursePreference`
Stores the preferences of instructors for optional courses, specifying which compulsory courses are important for students to have taken and with what weight.

**Example:**
- Optional Course: DataScience
  - Math: 70% importance
  - OOP: 30% importance

This means to be selected for DataScience, students' grades in Math and OOP are weighted as 70% and 30% respectively.

**Fields:**
- `optionalCourse`: The optional course for which preference is specified
- `compulsoryCourseCode`: Code of the compulsory course (e.g., "Math", "OOP")
- `percentage`: Weight importance (0-100)

#### REST Endpoints

**Create a Single Preference:**
```
POST /api/instructor-preferences
Content-Type: application/json

{
  "optional_course_id": 5,
  "compulsory_course_code": "Math",
  "percentage": 70.0
}
```

**Create Batch Preferences (Replace all preferences for a course):**
```
POST /api/instructor-preferences/batch
Content-Type: application/json

{
  "optional_course_id": 5,
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

**Get Preferences for a Course:**
```
GET /api/instructor-preferences/course/{courseId}
```

**Get Preferences by Course Code:**
```
GET /api/instructor-preferences/course-code/{courseCode}
```

**Update a Preference:**
```
PUT /api/instructor-preferences/{preferenceId}
Content-Type: application/json

{
  "optional_course_id": 5,
  "compulsory_course_code": "Math",
  "percentage": 75.0
}
```

**Delete a Preference:**
```
DELETE /api/instructor-preferences/{preferenceId}
```

**Delete All Preferences for a Course:**
```
DELETE /api/instructor-preferences/course/{courseId}/all
```

### 2. StableMatch Service Enhanced REST API

The StableMatch microservice has been enhanced with additional endpoints for querying assignment results:

**Solve Matching Problem:**
```
POST /api/v1/matching/solve
Content-Type: application/json

{
  "students": [...],
  "courses": [...],
  "capacity_per_course": 1
}
```

**Get All Assignments:**
```
GET /api/v1/matching/assignments
```

**Get Assignment for a Specific Student:**
```
GET /api/v1/matching/assignments/student/{studentId}
```

**Get All Assignments for a Course:**
```
GET /api/v1/matching/assignments/course/{courseId}
```

**Get Unmatched Students:**
```
GET /api/v1/matching/unmatched-students
```

**Get Full Courses:**
```
GET /api/v1/matching/full-courses
```

**Get Result Summary:**
```
GET /api/v1/matching/result-summary

Response:
{
  "status": "SUCCESS",
  "message": "...",
  "total_assignments": 150,
  "unmatched_count": 5,
  "full_courses_count": 10,
  "execution_time_ms": 250
}
```

### 3. Resilience Patterns Implementation

The PrefSchedule application implements three critical resilience patterns when communicating with the StableMatch service:

#### **Retry Pattern**
- **Max Attempts**: 3 (configurable)
- **Initial Delay**: 500ms (configurable)
- **Max Backoff**: 2 seconds
- **Exceptions to Retry**: SocketTimeoutException, IOException, ResourceAccessException

```yaml
resilience4j:
  retry:
    instances:
      stableMatchRetry:
        max-attempts: 3
        wait-duration: 500
        retry-exceptions:
          - java.net.SocketTimeoutException
          - java.io.IOException
          - org.springframework.web.client.ResourceAccessException
```

#### **Timeout Pattern**
- **Duration**: 5 seconds (configurable)
- **Cancel Running Future**: true

```yaml
resilience4j:
  timelimiter:
    instances:
      stableMatchTimeout:
        timeout-duration: 5s
        cancel-running-future: true
```

#### **Fallback Pattern**
When both Retry and Timeout patterns fail, the system falls back to a **random matching algorithm**:

```
StableMatch Service ──> SUCCESS ──> Return stable matching result
                 │
                 └──> RETRY (3 times) ──> TIMEOUT
                                          │
                                          └──> FALLBACK ──> Random Matching
                                                (RandomMatchingService)
```

The random matching service provides basic functionality:
- Randomly assigns each student to a course
- Respects course capacity constraints
- Returns unmatched students and full courses

### 4. Random Matching Service

**Service**: `RandomMatchingService`

Implements a fallback matching algorithm that works when the stable matching is unavailable:

```java
StableMatchingResponseDTO result = randomMatchingService.generateRandomMatching(request);
```

**Characteristics:**
- Disregards all preferences (as per requirements)
- Respects course capacity constraints
- Completes quickly for fallback purposes
- Returns the same response format as StableMatch service

### 5. Course Assignment Orchestration

**Service**: `CourseAssignmentOrchestrationService`

Orchestrates the complete assignment workflow:

1. **Batch Processing**: Groups optional courses into batches
2. **Score Calculation**: For each batch, calculates student scores based on:
   - Instructor preferences (which compulsory courses matter)
   - Student grades in those compulsory courses
   - Weighted average calculation
3. **Preference Building**: 
   - Student preferences: ordered list of all courses in batch
   - Course preferences: students ranked by their calculated scores
4. **Service Invocation**: Calls StableMatch service with built preferences
5. **Result Persistence**: Saves assignments to database as enrollments

#### REST Endpoint

**Execute Assignment Workflow:**
```
POST /api/assignments/execute-workflow?batchSize=5
Authorization: Bearer {admin_token}

Response:
{
  "status": "COMPLETED",
  "message": "Assignment workflow executed successfully",
  "total_batches": 3,
  "successful_batches": 3,
  "execution_time_ms": 5250,
  "results": {
    "0": { /* batch 0 result */ },
    "1": { /* batch 1 result */ },
    "2": { /* batch 2 result */ }
  }
}
```

## Configuration

### PrefSchedule Application Configuration

```yaml
# StableMatch service configuration
stable-match:
  url: http://localhost:8080/api
  timeout: 5000
  retry:
    max-attempts: 3
    delay: 500

# Resilience4j configuration
resilience4j:
  retry:
    instances:
      stableMatchRetry:
        max-attempts: 3
        wait-duration: 500
        retry-exceptions:
          - java.net.SocketTimeoutException
          - java.io.IOException
          - org.springframework.web.client.ResourceAccessException
  timelimiter:
    instances:
      stableMatchTimeout:
        timeout-duration: 5s
        cancel-running-future: true
```

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    PrefSchedule Service                      │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  CourseAssignmentOrchestrationService                   │ │
│  │  ├─ Batch optional courses                              │ │
│  │  ├─ Calculate student scores based on:                  │ │
│  │  │  ├─ InstructorCoursePreferences                      │ │
│  │  │  └─ Student Grades                                   │ │
│  │  ├─ Build preferences (students & courses)              │ │
│  │  └─ Invoke StableMatch service in batches               │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                    │
│                          ▼                                    │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  StableMatchClient (with Resilience4j)                  │ │
│  │  ├─ Retry (3 attempts, 500ms delay)                     │ │
│  │  ├─ Timeout (5 seconds)                                 │ │
│  │  └─ Fallback → RandomMatchingService                    │ │
│  └────────────────────────────────────────────────────────┘ │
│           │                                  │                │
│           │ (Success)                        │ (Fallback)     │
│           ▼                                  ▼                │
│  ┌─────────────────────────┐      ┌────────────────────┐   │
│  │  StableMatch Service    │      │ RandomMatching     │   │
│  │  (Stable Matching       │      │ Service            │   │
│  │   Algorithm)            │      │ (Random Algorithm) │   │
│  └─────────────────────────┘      └────────────────────┘   │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Data Flow Example

### Example 1: Basic Setup

**Optional Course: "Advanced Data Science"**

Instructor specifies prerequisites importance:
```
POST /api/instructor-preferences/batch
{
  "optional_course_id": 10,
  "preferences": [
    {"compulsory_course_code": "Math", "percentage": 60},
    {"compulsory_course_code": "OOP", "percentage": 40}
  ]
}
```

### Example 2: Workflow Execution

**Trigger Assignment:**
```
POST /api/assignments/execute-workflow?batchSize=5
```

**Process Flow:**
1. System retrieves all optional courses
2. Groups them into batches of 5
3. For each batch:
   - Gets all students
   - Retrieves InstructorCoursePreference for "Advanced Data Science"
   - For each student, calculates score:
     ```
     Score = (Math_Grade × 0.60) + (OOP_Grade × 0.40)
     ```
   - Creates StableMatchingRequestDTO with:
     - Student preferences: [AdvancedDataScience, MachineLearning, DataMining]
     - Course preferences: students ranked by their calculated scores
   - Invokes StableMatch service with retry/timeout/fallback
   - Saves successful assignments to database

### Example 3: Resilience in Action

**Scenario: StableMatch service is temporarily unavailable**

```
1. First request to StableMatch → Connection refused
   └─ Retry 1: Wait 500ms, retry
   
2. Retry 1 → Connection refused
   └─ Retry 2: Wait 500ms-1s, retry
   
3. Retry 2 → Timeout (exceeds 5 seconds)
   └─ Fallback activated
   
4. RandomMatchingService returns:
   {
     "status": "SUCCESS",
     "message": "Random matching completed (StableMatch unavailable, used random fallback)",
     "assignments": [...],
     "unmatched_students": [...],
     "full_courses": [...],
     "execution_time_ms": 150
   }
```

## Integration Points

### With StableMatch Service
- Sends batches of optional courses with calculated student preferences
- Receives stable matching results for each batch
- Handles service unavailability gracefully

### With Grade System
- Reads student grades via `GradeRepository`
- Uses grades to calculate student scores for optional courses
- Filters by student code and compulsory course code

### With Enrollment System
- Saves assignment results as enrollments
- Checks for duplicate enrollments before saving

## Security

- All endpoints require appropriate role authorization:
  - **POST/PUT/DELETE instructor preferences**: INSTRUCTOR or ADMIN
  - **GET instructor preferences**: Any authenticated user
  - **Execute workflow**: ADMIN only

## Error Handling

- Validation of instructor preferences (0-100% range)
- Graceful fallback to random matching
- Detailed error messages in responses
- Comprehensive logging for debugging

## Future Enhancements

1. Database persistence for assignment history
2. Student preference input integration
3. Time-based scheduling for batch processing
4. Advanced preference weighting algorithms
5. Assignment reversal/modification endpoints
6. Analytics and reporting
