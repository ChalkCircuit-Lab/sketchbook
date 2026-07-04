Java Technologies - Lab 5
[valid 2025-2026]

RESTful Services
We continue our project.

Compulsory (1p)

      Implement CRUD REST endpoints for students.
      Test them using curl, or Postman.



Homework (2p)

      Extend the domain model with student preferences: each student must submit their preferences regarding
      all the courses from his year of study. The order relationship between courses id defined for each pack, and
      it may be partial. There may be ties between courses.
      Implement CRUD REST endpoints for collecting student preferences.
      Use Data Transfer Objects (DTOs) and Bean Validation.
      Create at least one custom runtime exception and handle it either at controller level, or globally.
      Implement conditional requests using ETag and support If-None-Match.
      Support content negotiation for JSON and XML for at least one resource.
      Document at least one endpoint with Springdoc OpenAPI and test your services through Swagger UI.



Advanced (2p)

      Add HATEOAS links to the student endpoints to enable discoverability of related resources (e.g., student
      preferences).
      Implement API versioning (URI or header-based) and document migration strategy.
      Add schema migrations using Flyway or Liquibase and include initial seed data.
