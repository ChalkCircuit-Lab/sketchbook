Java Technologies - Lab 8
[valid 2025-2026]

Microservices
We continue our project.

Compulsory (1p)

      Create a new Spring Boot project (StableMatch) dedicated to the algorithm of assigning students to
      optional courses.
      This application must be completely independent of the rest of the system.
      StableMatch will contain an endpoint having as input a stable matching problem in JSON format and will
      return the solution also as a JSON object.
      Design the request and response DTOs and create the REST controller.


Homework (2p)

      Create another table in the PrefSchedule application containing the preferences of the optional course
      instructors.
      The instructor of an optional course will specify zero or more pairs of type (compulsory course abbr,
      percentage) with the significance: how important is for students to have better grades at that compulsory
      course, in order to be assigned to their the optional course.
      Example: CO1: {(Math, 100%)}; CO2 {(OOP, 50%), (Java, 50%)}. For each optional course, the students
      will be ordered based on the weighted average of grades in the compulsory courses taken into account for
      the optional. in order.
      Create the necessary endpoints for the instructors' preferences in the PrefSchedule application.
      Create the necessary REST endpoints for StableMatch service (e.g., get all assignments, get for a student,
      etc.).
      Implement a random algorithm for creating a matching between students and optional courses
      (disregarding preferences).
      Implement the resilience patterns Retry, Fallback, Timeout.
      Invoke StableMatch service, from the PrefSchedule, for each pack of optional courses.


Advanced (2p)

      Implement Gale-Shapley algorithm for creating a stable matching between students and optional courses.
      Implement the resilience patterns CircuitBreaker, Bulkhead, RateLimiter. Design them in order to be able
      to show their behavior in case of failures.
      Use JMeter, or a similar tool, in order to implement stress tests that will trigger the resilience behavior of
      the services.
