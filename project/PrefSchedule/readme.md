### Project: PrefSchedule
***
GOAL: Design a system that automatically assigns students to optional courses based on their:
- preferences,
- course capacity,
- instructors' preferences.

The system should ensure a fair distribution of students using a `stable-matching algorithm`.

## 4. Java Persistence API
> Compulsory (1p)
- Create a Spring Boot Project that has support for Spring Data JPA, and PostgreSQL.
- Using an SQL script, create tables for:
  - students (id, code, name, email, year)
  - instructors (id, name, email)
  - packs (id, year, semester, name)
  - courses (id, type, code, abbr, name, instructor_id, pack_id, group_count, description)
- A course can be compulsory or optional. 
- Optional courses are grouped in packs. 
- A student will be assigned exactly one course from each pack in their year.
- Create the entity class and a repository for students.
- Implement a simple test using a CommandLineRunner.

> Homework (2p)
- Create all the entity classes.
- Use @OneToMany and @ManyToOne associations. (You may use Lombok, but it is not necessarily required.)
- Think carefully if Student and Instructor should inherit a base abstract class.
- Create Spring JPA repositories for all entities.
- Think what queries may be necessary and define them in the repos:
  - use at least one query based on a specific JPQL string,
  - one derived query,
  - one transactional, modifying query.
- Create a @Service class for each entity, that uses the corresponding repo.
- Use Java Faker to populate the database and test CRUD operations for courses.

***
## 5. RESTful Services

> Compulsory (1p)
- Implement CRUD REST endpoints for students.
- Test them using curl, or Postman.

> Homework (2p)
- Extend the domain model with student preferences:
  - each student must submit their preferences regarding all the courses from his year of study.
  - The order relationship between courses id defined for each pack, and it may be partial.
  - There may be ties between courses.
- Implement CRUD REST endpoints for collecting student preferences.
- Use Data Transfer Objects (DTOs) and Bean Validation.
- Create at least one custom runtime exception and handle it either at controller level, or globally.
- Implement conditional requests using ETag and support If-None-Match.
- Support content negotiation for JSON and XML for at least one resource.
- Document at least one endpoint with Springdoc OpenAPI and test your services through Swagger UI.

***

## 6. Security
>Compulsory (1p)
- Integrate Spring Security and verify that all your endpoints are now protected.
- Create a mock /login endpoint.
- Configure the security chain such as to permit unauthenticated requests only to the /login endpoint.

> Homework (2p)
- Refactor your domain such as students and instructors are also users of the application.
- Other users may exist, such as an administrator.
- Configure Spring Boot to read the users and their roles (ADMIN, INSTRUCTOR, STUDENT) from the database.
- Implement JWT-based authentication and a login endpoint issuing tokens.
- Protect POST/PUT/DELETE endpoints with role-based access and keep GET endpoints public where reasonable.
- Implement password storage with BCrypt and a user registration flow.
- Add method-level security with @PreAuthorize and test it.
- Secure Actuator endpoints: expose only health and info publicly; require auth for metrics.

*** 

## 7. Messaging
> Compulsory (1p)
- Install a messaging broker such as Kafka, ActiveMQ, or RabbitMQ.
- Create another Spring Boot project (QuickGrade) dedicated to student grades.
- The QuickGrade application must publish events to a topic, containing (student code, course code, grade).
- PrefSchedule will consume the messages and print them to the console.
> Homework (2p)
- Create another table in the PrefSchedule application containing student grades.
- The grades received as events from QuickGrade must be stored in the database - only those related to compulsoy courses.
- Create REST endpoints for getting the grades and loading them from a CSV file.
- Implement a Dead-Letter Queue (DLQ) handler for failed messages and test retry semantics.

***

## 8. Microservices

> Compulsory (1p)
- Create a new Spring Boot project (StableMatch) dedicated to the algorithm of assigning students to optional courses.
- This application must be completely independent of the rest of the system.
- StableMatch will contain an endpoint having as input a stable matching problem in JSON format and will return the solution also as a JSON object.
- Design the request and response DTOs and create the REST controller.
> Homework (2p)
- Create another table in the PrefSchedule application containing the preferences of the optional course instructors.
- The instructor of an optional course will specify zero or more pairs of type (compulsory course abbr, percentage) with the significance: how important is for students to have better grades at that compulsory course, in order to be assigned to their the optional course.
- Example:
  - CO1: {(Math, 100%)};
  - CO2 {(OOP, 50%), (Java, 50%)}.
- For each optional course, the students will be ordered based on the weighted average of grades in the compulsory courses taken into account for the optional. in order.
- Create the necessary endpoints for the instructors' preferences in the PrefSchedule application.
- Create the necessary REST endpoints for StableMatch service (e.g., get all assignments, get for a student, etc.).
- Implement a random algorithm for creating a matching between students and optional courses (disregarding preferences).
- Implement the resilience patterns Retry, Fallback, Timeout.
- Invoke StableMatch service, from the PrefSchedule, for each pack of optional courses.
