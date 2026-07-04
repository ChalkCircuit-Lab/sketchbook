# Java Technologies - Lab 3

## Dependency Injection and Cross-Cutting Concerns

### Compulsory Work (1p)

#### Objective

Create a simple, illustrative Spring Boot project that demonstrates all major types of **Dependency Injection (DI)** and proves the order of their execution:

```
Constructor → Field → Setter & Method
```

---

### 1. Theoretical Summary

**Inversion of Control (IoC)** is a principle stating that objects should not create or manage their dependencies directly. Instead, an external container provides them.

**Dependency Injection (DI)** is a design technique that implements IoC by having the framework inject the required dependencies into the components. In Spring, this process is managed by the **IoC container**.

**Benefits:**

* Reduces coupling between components.
* Improves flexibility, testability, and maintainability.
* Promotes separation of concerns.

**Injection Points:**

1. **Constructor Injection** – recommended for mandatory dependencies.
2. **Field Injection** – simplest form but less testable.
3. **Setter or Method Injection** – suitable for optional dependencies.

**Execution Order:**
Spring initializes beans in the following sequence:

```
1. Constructor Injection
2. Field Injection
3. Setter / Method Injection
```

---

### 2. Implementation Steps

#### Step 1. Create a Spring Boot project

* Use [https://start.spring.io](https://start.spring.io)
* Project: **Maven**, Language: **Java**, Spring Boot: **3.2+**
* Group: `ro.uaic.dbxdrgsl`
* Artifact: `Lab03DI`
* Dependencies:

  * Spring Web
  * DevTools
  * Actuator

Generate, unzip, and open the project in IntelliJ or VS Code.

#### Step 2. Folder structure

```
src/main/java/ro/uaic/dbxdrgsl/lab03di/
  ├─ Lab03DiApplication.java
  ├─ service/
  │   ├─ MessageService.java
  │   └─ TimeService.java
  └─ controller/
      └─ DemoController.java
```

#### Step 3. Create service beans

**MessageService.java**

```java
@Service
public class MessageService {
    public String getMessage() {
        return "Injected message from MessageService";
    }
}
```

**TimeService.java**

```java
@Service
public class TimeService {
    public String getCurrentTime() {
        return "Current time: " + LocalTime.now();
    }
}
```

#### Step 4. Create controller demonstrating DI order

**DemoController.java**

```java
@RestController
public class DemoController {

    private final MessageService messageService; // Constructor injection

    @Autowired
    private TimeService timeService; // Field injection

    private String extraInfo;

    @Autowired
    public DemoController(MessageService messageService) {
        System.out.println("1. Constructor Injection");
        this.messageService = messageService;
    }

    @Autowired
    public void init() {
        System.out.println("2. Field Injection");
    }

    @Autowired
    public void setExtraInfo() {
        System.out.println("3. Setter/Method Injection");
        this.extraInfo = "Setter injection executed";
    }

    @GetMapping("/demo")
    public String showInjectionOrder() {
        return String.format(
            "<h2>Dependency Injection Order Demo</h2><p>1. %s</p><p>2. %s</p><p>3. %s</p>",
            messageService.getMessage(),
            timeService.getCurrentTime(),
            extraInfo
        );
    }
}
```

#### Step 5. Run application

```bash
mvn spring-boot:run
```

**Expected console log:**

```
1. Constructor Injection
2. Field Injection
3. Setter/Method Injection
```

#### Step 6. Test in browser

Open: [http://localhost:8080/demo](http://localhost:8080/demo)

Result:
Displays output from all injected beans and confirms injection order.

---

### 3. Key Concepts from Lecture

| Concept                           | Description                                                  |
| --------------------------------- | ------------------------------------------------------------ |
| **IoC Container**                 | Manages object lifecycle and injections.                     |
| **@Autowired / @Inject**          | Annotations marking injection points.                        |
| **@Component / @Service / @Bean** | Marks classes or methods as beans managed by Spring.         |
| **@Qualifier / @Primary**         | Resolves ambiguity when multiple beans match a dependency.   |
| **Scopes**                        | Define bean lifetimes (singleton, prototype, request, etc.). |
| **Lifecycle Annotations**         | `@PostConstruct`, `@PreDestroy` handle resource management.  |

---

### 4. Verification Checklist

* [x] Project builds successfully.
* [x] Console prints injection order.
* [x] `/demo` endpoint returns all expected values.
* [x] Code demonstrates constructor, field, and setter injection.

---

### 5. References

* Java Technologies Lecture 3: *Beans, Dependency Injection and Cross-Cutting Concerns* (Fall 2025)
* Spring Framework Documentation: [https://docs.spring.io/spring-framework/docs/current/reference/html/core.html#beans](https://docs.spring.io/spring-framework/docs/current/reference/html/core.html#beans)

---

**End of README**
