## Java Technologies – Lab 2 (2025–2026)

### Spring Boot Application with Multiple Profiles

---

### 1. Compulsory (1p)

#### **Requirements**

1. Create a Spring Boot application using Spring Initializr.
2. Add dependencies: **DevTools**, **Spring Web**.
3. Change Tomcat port to **8081**.
4. Add an endpoint `/hello` returning a greeting message defined in `application.properties`.
5. Add **Spring Boot Actuator** support and test endpoints.

#### **Solution Description**

The compulsory task demonstrates basic Spring Boot setup, property injection, and actuator monitoring.

* **Spring Boot Initializer:** Used to generate the project scaffold with Maven, Java 17, and dependencies.
* **Server Port Change:** Done by setting `server.port=8081` in `application.properties`.
* **Greeting Message:** Stored in the same file as `greeting.message=Hello from Spring Boot!` and injected using `@Value`.
* **Controller:** A `HelloController` exposes `/hello` endpoint returning the greeting.
* **Actuator:** Added to `pom.xml` and exposed through `management.endpoints.web.exposure.include=*`.

#### **Implementation Steps**

1. Generate project via [start.spring.io](https://start.spring.io) with Web, DevTools, and Actuator.
2. Modify `application.properties`:

   ```properties
   server.port=8081
   greeting.message=Hello from Spring Boot running on port 8081!
   management.endpoints.web.exposure.include=*
   ```
3. Create controller:

   ```java
   @RestController
   public class HelloController {
       @Value("${greeting.message}")
       private String greeting;

       @GetMapping("/hello")
       public String sayHello() { return greeting; }
   }
   ```
4. Run `mvn spring-boot:run` and verify:

    * `/hello` returns greeting message.
    * `/actuator` and `/actuator/health` work.

---

### 2. Homework (2p)

#### **Requirements Recap**

* Same application must support multiple environments: **dev** and **prod**.
* Each profile uses a different database configured in `application-{profile}.yml`.
* Use **JDBC** for database access.
* Use **Spring Expression Language (SpEL)** with conditional beans.
* Use environment variables for DB host, port, name, user, password.
* Choose between `@ConfigurationProperties` or `@Value` and justify.
* Print table data at startup using `CommandLineRunner`.
* Show that command-line arguments override YAML values.
* Combine `@Profile` and `@ConditionalOnExpression` to choose DB.
* Create a custom **Actuator InfoContributor** to display DB details.

#### **Theoretical Background**

* **Spring Profiles:** Enable different configurations for dev/test/prod environments via `application-{profile}.yml`.
* **Environment Variables:** Allow externalized configuration, making deployments portable.
* **@ConfigurationProperties:** Binds hierarchical config structures into typed POJOs. Preferred for structured config.
* **@Profile:** Activates beans conditionally based on the active environment profile.
* **@ConditionalOnExpression:** Adds dynamic conditions (using SpEL) for bean creation.
* **CommandLineRunner:** Executes code right after context initialization.
* **Actuator InfoContributor:** Adds custom runtime info under `/actuator/info`.

#### **Implementation Description**

1. **Profiles Setup**

    * Default profile is `dev`, configured in `application.yml`.
    * Each profile has its own YAML file (`application-dev.yml`, `application-prod.yml`).
    * Dev uses H2 in-memory DB; Prod uses MySQL.

2. **Environment Variables**
   Example on Windows PowerShell:

   ```powershell
   $env:APP_DB_VENDOR="mysql"
   $env:APP_DB_HOST="localhost"
   $env:APP_DB_PORT="3306"
   $env:APP_DB_NAME="proddb"
   $env:APP_DB_USER="springuser"
   $env:APP_DB_PASSWORD="springpass"
   ```

3. **Config Properties Binding**
   `AppDbProperties.java` binds these variables using `@ConfigurationProperties(prefix="app.db")`.
   This is better than `@Value` because it groups related settings, improves maintainability, and supports validation.

4. **Conditional Bean Creation**
   `DataSourceConfig.java` defines two `DataSource` beans:

    * One active under `@Profile("dev")` when vendor is H2.
    * One active under `@Profile("prod")` when vendor is MySQL.
      Each condition uses `@ConditionalOnExpression` with a SpEL check on the vendor property.

5. **Database and JDBC Repository**

    * H2 automatically initializes from `schema.sql` and `data.sql`.
    * MySQL is manually created using:

      ```sql
      CREATE DATABASE proddb;
      CREATE USER 'springuser'@'localhost' IDENTIFIED BY 'springpass';
      GRANT ALL PRIVILEGES ON proddb.* TO 'springuser'@'localhost';
      CREATE TABLE students (id INT PRIMARY KEY, name VARCHAR(50));
      INSERT INTO students VALUES (10, 'Prod-Alice'), (11, 'Prod-Bob');
      ```
    * `StudentRepository` uses `JdbcTemplate` for `SELECT` queries.

6. **Startup Validation**
   `DatabasePrinter` implements `CommandLineRunner` and prints active profile, DB connection details, and table rows at startup.

7. **Command-Line Precedence**
   Run examples:

   ```bash
   mvn spring-boot:run -Dspring-boot.run.arguments="--server.port=8090"
   mvn spring-boot:run -Dspring-boot.run.arguments="--greeting.message=CLI wins"
   ```

   These override YAML properties, proving Spring Boot's property precedence.

8. **Actuator InfoContributor**
   `DatabaseInfoContributor` adds DB details to `/actuator/info` endpoint:

   ```json
   {
     "database": {
       "vendor": "mysql",
       "host": "localhost",
       "port": 3306,
       "name": "proddb",
       "user": "springuser",
       "activeProfiles": "prod"
     }
   }
   ```

#### **What to Show in the Report / Demo**

* **Project structure** with `config`, `controller`, `repository`, and `actuator` packages.
* **Console logs** for dev and prod runs:

    * Active profile
    * Database vendor and connection string
    * Query results (`Alice`, `Bob`, etc.)
* **Screenshots:**

    * `/hello` endpoint output
    * `/actuator/info` JSON data for both profiles
    * CLI override example (`port=8090`, custom greeting)
* **Motivation Summary:** `@ConfigurationProperties` chosen for structured binding; profiles ensure environment-specific configuration; SpEL adds flexibility.

---

### 3. Summary

| Feature                 | Implementation                                | Proof                             |
| ----------------------- | --------------------------------------------- | --------------------------------- |
| Custom port             | `server.port=8081`                            | `/hello` works on 8081            |
| Property injection      | `@Value` in HelloController                   | Message visible                   |
| Profiles                | `application-dev.yml`, `application-prod.yml` | Console logs                      |
| Environment variables   | APP_DB_*                                      | Displayed by runner               |
| Conditional Beans       | `@Profile` + `@ConditionalOnExpression`       | Logs show DataSource type         |
| JDBC Access             | `JdbcTemplate`                                | Printed rows                      |
| Command-line precedence | `--server.port`, `--greeting.message`         | Overrides visible                 |
| Actuator extension      | `DatabaseInfoContributor`                     | `/actuator/info` shows DB details |

---

### Final Outcome

A single Spring Boot application capable of:

* Running on multiple environments (dev, prod)
* Reading DB credentials dynamically from environment
* Automatically connecting to the correct DB
* Demonstrating property injection, profiles, and conditional configuration
* Providing custom runtime monitoring via Actuator
