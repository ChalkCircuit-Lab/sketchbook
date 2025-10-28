# Java Technologies - Lab 3 (Advanced)

### Drools Rule Engine & Spring Boot Starter (Auditing)

---

## **Objective**

Design a maintainable, rule-based discount system using **Drools** and implement a reusable **Spring Boot Starter** for cross-cutting auditing.

---

## **1. Context and Theory**

### 1.1. Motivation

When business rules (discounts, pricing, eligibility) grow in complexity, hardcoding them in Java services leads to rigid, unmaintainable code. A **rule engine** externalizes these decisions into declarative rules.

### 1.2. Drools Overview

Drools is a **Business Rule Management System (BRMS)** implementing the Rete algorithm for pattern matching. It separates business logic (rules) from application code.

**Key Terms:**

* **KieContainer:** Loads the knowledge base.
* **KieSession:** Executes the rules.
* **DRL File (.drl):** Contains declarative business rules.
* **Facts:** Java objects inserted into the working memory.

### 1.3. Advantages

* Non-developers can modify business rules.
* Dynamic rule evaluation at runtime.
* Eliminates long `if-else` chains.
* Maintains separation of concerns.

---

## **2. Project Setup**

**Folder:** `Lab03/Advanced/DroolsDiscounts`

**Dependencies (pom.xml):**

```xml
<dependency>
  <groupId>org.kie</groupId>
  <artifactId>kie-spring</artifactId>
  <version>8.44.0.Final</version>
</dependency>
<dependency>
  <groupId>org.kie</groupId>
  <artifactId>drools-decisiontables</artifactId>
  <version>8.44.0.Final</version>
</dependency>
```

**Structure:**

```
DroolsDiscounts/
 ├─ model/
 │   ├─ Customer.java
 │   └─ Order.java
 ├─ service/
 │   ├─ DiscountService.java
 │   └─ DroolsDiscountService.java
 ├─ controller/OrderController.java
 ├─ config/DroolsConfig.java
 ├─ audit/AuditingAspect.java
 ├─ resources/rules/discount-rules.drl
 └─ META-INF/kmodule.xml
```

---

## **3. Implementation Steps**

### 3.1. Domain Model

```java
public class Customer {
    private String name;
    private boolean loyal;
    // constructor, getters
}

public class Order {
    private double total;
    private double discount;
    // constructor, getters, setters
}
```

### 3.2. Drools Configuration

```java
@Configuration
public class DroolsConfig {
    @Bean
    public KieContainer kieContainer() {
        return KieServices.Factory.get().getKieClasspathContainer();
    }

    @Bean
    public KieSession kieSession(KieContainer kieContainer) {
        return kieContainer.newKieSession("discountSession");
    }
}
```

### 3.3. Rules Definition — `discount-rules.drl`

```drl
package rules
import ro.uaic.dbxdrgsl.droolsdiscounts.model.*;

rule "Loyal customer discount"
    when
        $c : Customer( loyal == true )
        $o : Order()
    then
        $o.setDiscount($o.getTotal() * 0.10);
        System.out.println("[DROOLS] Loyalty discount applied.");
end

rule "Large order discount"
    when
        $c : Customer( loyal == false )
        $o : Order( total > 500 )
    then
        $o.setDiscount(50);
        System.out.println("[DROOLS] Large order discount applied.");
end

rule "No discount"
    when
        $c : Customer( loyal == false )
        $o : Order( total <= 500 )
    then
        $o.setDiscount(0);
        System.out.println("[DROOLS] No discount.");
end
```

### 3.4. Service Layer

```java
@Service
public class DroolsDiscountService implements DiscountService {
    private final KieSession kieSession;
    public DroolsDiscountService(KieSession kieSession) { this.kieSession = kieSession; }

    @Override
    public double applyDiscount(Customer c, Order o) {
        kieSession.insert(c);
        kieSession.insert(o);
        kieSession.fireAllRules();
        return o.getDiscount();
    }
}
```

### 3.5. REST Controller

```java
@RestController
@RequestMapping("/orders")
public class OrderController {
    private final DiscountService discountService;

    public OrderController(DiscountService discountService) { this.discountService = discountService; }

    @GetMapping("/test")
    public String test(@RequestParam String name, @RequestParam boolean loyal, @RequestParam double total) {
        Customer c = new Customer(name, loyal);
        Order o = new Order(total);
        double d = discountService.applyDiscount(c, o);
        return String.format("%s gets %.2f discount", name, d);
    }
}
```

### 3.6. KIE Module Definition

`src/main/resources/META-INF/kmodule.xml`

```xml
<kmodule xmlns="http://www.drools.org/xsd/kmodule">
  <kbase name="discountKBase" packages="rules">
    <ksession name="discountSession"/>
  </kbase>
</kmodule>
```

---

## **4. Auditing Aspect and Starter**

### 4.1. Auditing Aspect

```java
@Aspect
@Component
public class AuditingAspect {
    @AfterReturning(pointcut = "execution(* ro.uaic.dbxdrgsl.droolsdiscounts.service.DiscountService.applyDiscount(..))",
                    returning = "discount")
    public void log(JoinPoint jp, Object discount) {
        System.out.printf("[AUDIT] %s executed, discount: %.2f%n", jp.getSignature().getName(), discount);
    }
}
```

### 4.2. Optional Reusable Starter

Create a new module `AuditStarter` with:

```
META-INF/spring/org.springframework.boot.autoconfigure.AutoConfiguration.imports
```

and class:

```java
@AutoConfiguration
public class AuditAutoConfiguration {
    @Bean
    public AuditingAspect auditingAspect() {
        return new AuditingAspect();
    }
}
```

This auto-loads auditing in any project including this JAR.

---

## **5. Execution & Testing**

**Run:**

```bash
mvn spring-boot:run
```

**Test URLs:**

1. Loyalty discount:

   ```
   ```

[http://localhost:8080/orders/test?name=Alice&loyal=true&total=300](http://localhost:8080/orders/test?name=Alice&loyal=true&total=300)

```
2. Large order discount:
```

[http://localhost:8080/orders/test?name=Bob&loyal=false&total=600](http://localhost:8080/orders/test?name=Bob&loyal=false&total=600)

```
3. No discount:
```

[http://localhost:8080/orders/test?name=Eve&loyal=false&total=100](http://localhost:8080/orders/test?name=Eve&loyal=false&total=100)

```

**Expected console:**
```

[DROOLS] Loyalty discount applied.
[SERVICE] Discount for Alice = 30.00
[AUDIT] applyDiscount executed, discount: 30.00

```

---

## **6. Deliverables for Submission**

| Requirement | Evidence |
|--------------|-----------|
| Drools rules loaded and executed | Console output + DRL file |
| Profiles or dynamic rule logic | Screenshot of running session |
| Auditing aspect working | Console audit messages |
| Optional starter JAR | Config + dependency demonstration |

---

## **7. Concept Recap (from Lecture)**
- **IoC**: Inversion of Control delegates dependency management to Spring.
- **DI Types**: Constructor (mandatory), Field (simple), Setter (optional).
- **AOP**: Used for cross-cutting logic (e.g., auditing, logging).
- **Eventing**: Enables loose coupling between services.
- **Profiles / Alternatives**: Used to select bean implementations dynamically.

---

## **8. Conclusion**
This advanced lab integrates **Drools** with **Spring Boot** to externalize decision logic, and demonstrates cross-cutting auditing through **AOP** and **AutoConfiguration**. Together, they illustrate clean separation of business rules and technical concerns, fulfilling maintainability and modularity principles.

```
