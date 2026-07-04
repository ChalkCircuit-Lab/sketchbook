# StableMatch - Stable Matching Algorithm Service

A Spring Boot application dedicated to solving stable matching problems for assigning students to optional courses using the Gale-Shapley algorithm.

## Overview

StableMatch is a completely independent microservice that provides a REST API for solving stable matching problems. It accepts a stable matching problem specification in JSON format containing student and course preferences, and returns optimal assignments ensuring stability.

## Features

- **Gale-Shapley Algorithm**: Implements a variant optimized for many-to-one matching (multiple students per course)
- **Flexible Capacity**: Support for configurable capacity per course (default: 1)
- **Preference-Based Matching**: Both students and courses provide preference rankings
- **Comprehensive Response**: Returns assignments with preference rankings, unmatched students, and execution metrics
- **Input Validation**: Robust validation of request data
- **REST API**: Clean, well-documented endpoints
- **Logging**: Detailed logging for monitoring and debugging

## Project Structure

```
StableMatch/
├── pom.xml                          # Maven configuration
├── .mvn/wrapper/                    # Maven wrapper files
├── src/
│   ├── main/
│   │   ├── java/com/stablematch/
│   │   │   ├── StableMatchApplication.java       # Main Spring Boot app
│   │   │   ├── algorithm/
│   │   │   │   └── StableMatchingService.java    # Core algorithm service
│   │   │   ├── controller/
│   │   │   │   └── StableMatchingController.java # REST controller
│   │   │   └── dto/
│   │   │       ├── StableMatchingRequestDTO.java # Request DTO
│   │   │       └── StableMatchingResponseDTO.java # Response DTO
│   │   └── resources/
│   │       └── application.properties            # Configuration
│   └── test/
│       └── java/com/stablematch/
│           └── algorithm/
│               └── StableMatchingServiceTest.java # Unit tests
└── README.md
```

## Technology Stack

- **Spring Boot 3.2.0** - Web framework
- **Java 17** - Programming language
- **Lombok** - Boilerplate reduction
- **Jackson** - JSON serialization
- **JUnit 5** - Testing framework

## Configuration

Default configuration in `application.properties`:
```properties
spring.application.name=stable-match
server.port=8080
server.servlet.context-path=/api
logging.level.com.stablematch=INFO
```

## API Endpoints

### Health Check
```
GET /api/v1/matching/health
```

### API Information
```
GET /api/v1/matching/info
```

### Solve Stable Matching
```
POST /api/v1/matching/solve
Content-Type: application/json
```

## Request Format

```json
{
  "students": [
    {
      "student_id": "S1",
      "preferences": ["C1", "C2", "C3"]
    },
    {
      "student_id": "S2",
      "preferences": ["C2", "C1", "C3"]
    }
  ],
  "courses": [
    {
      "course_id": "C1",
      "preferences": ["S1", "S2"]
    },
    {
      "course_id": "C2",
      "preferences": ["S2", "S1"]
    }
  ],
  "capacity_per_course": 1
}
```

## Response Format

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
  "execution_time_ms": 15
}
```

## Building and Running

### Build the Project
```bash
mvn clean package
```

### Run the Application
```bash
mvn spring-boot:run
```

Or after building:
```bash
java -jar target/stable-match-1.0.0.jar
```

The service will start on `http://localhost:8080/api`

## Running Tests

```bash
mvn test
```

## Example Usage

### Using cURL

```bash
curl -X POST http://localhost:8080/api/v1/matching/solve \
  -H "Content-Type: application/json" \
  -d '{
    "students": [
      {"student_id": "S1", "preferences": ["C1", "C2"]},
      {"student_id": "S2", "preferences": ["C2", "C1"]}
    ],
    "courses": [
      {"course_id": "C1", "preferences": ["S1", "S2"]},
      {"course_id": "C2", "preferences": ["S2", "S1"]}
    ],
    "capacity_per_course": 1
  }'
```

### Using Java

```java
StableMatchingRequestDTO request = StableMatchingRequestDTO.builder()
    .students(Arrays.asList(
        StudentPreference.builder()
            .studentId("S1")
            .preferences(Arrays.asList("C1", "C2"))
            .build()
    ))
    .courses(Arrays.asList(
        CoursePreference.builder()
            .courseId("C1")
            .preferences(Arrays.asList("S1"))
            .build()
    ))
    .capacityPerCourse(1)
    .build();

StableMatchingResponseDTO response = stableMatchingService.solveStableMatching(request);
```

## Algorithm Details

The implementation uses a variant of the Gale-Shapley algorithm optimized for many-to-one matching:

1. **Initialization**: All students are initially free
2. **Main Loop**: While free students exist:
   - Each free student proposes to their next preferred course
   - If the course has capacity, the student is assigned
   - If the course is full, it compares the new student with its worst current match
   - If the new student is preferred, the worst student is freed and the new student is assigned
3. **Termination**: When all students are either assigned or have exhausted their preferences

## Error Handling

The API handles various error cases:
- Empty student or course lists
- Null or duplicate IDs
- Students or courses with no preferences
- Duplicate student or course IDs

All errors are returned with a clear error status and message in the response.

## Notes

- This is a completely independent service
- No external dependencies on other modules
- Scales to handle problems with hundreds of students and courses
- Execution time is tracked and included in the response

## License

Part of the JavaTech educational project.
