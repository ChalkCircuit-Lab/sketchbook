# Java Technologies - Lab 3

## Dependency Injection and Cross-Cutting Concerns

### 📘 Theoretical Background (from Lecture 3: *Beans, Dependency Injection, and Cross-Cutting Concerns*)

#### Inversion of Control (IoC)

* Principle: A class should not control its own dependencies; instead, an external framework (the IoC container) provides them.
* This removes tight coupling and improves flexibility and testability.
* Also known as the *Hollywood Principle*: "Don’t call us, we’ll call you."

#### Dependency Injection (DI)

* A design pattern that implements IoC by passing dependencies into a class rather than instantiating them internally.
* Types of injection points:

  1. **Constructor Injection** (recommended) – for mandatory dependencies.
  2. **Field Injection** – easiest, but less clean; hard to test.
  3. **Setter/Method Injection** – for optional dependencies.

#### Spring Beans and the IoC Container

* **Bean**: an object managed by the Spring container.
* Created via annotations like `@Component` or configuration via `@Bean`.
* Lifecycle managed by `ApplicationContext`, which also handles events and dependency resolution.

#### Cross-Cutting Concerns (AOP)

* Concerns like logging, transactions, or security are not tied to one specific class.
* Implemented via **Aspects** (modules that intercept method calls at defined join points).
* Implemented using Spring AOP and annotations such as `@Aspect`, `@Before`, `@Around`, etc.

#### Event Publishing

* Spring enables decoupled communication between components using events.
* Components can publish domain events (e.g., `OrderCreatedEvent`) and others can listen using `@EventListener`.

#### Profiles and Alternatives

* `@Profile` allows multiple beans of the same type, each active under different configurations (e.g., `dev`, `prod`, `test`).
* Simplifies switching between different business logic implementations.

---

## 🧮 Homework Description

### Task

Create a **Spring Boot Order Management System** that:

1. Handles Customers, Orders, and Discounts.
2. Supports three discount policies:

   * **Loyal customers** get a percentage discount.
   * **Large orders** get a fixed amount discount.
   * **Default:** no discount.
3. Uses Spring Profiles to select the active discount policy.
4. Logs each discount applied (method, customer, and discount amount).
5. Adds an **Aspect** to validate eligibility before applying a discount.
6. Publishes an **event** whenever a discount exceeds a threshold, and a listener reacts to it.

---

## 🧩 Step-by-Step Implementation Guide

### 1️⃣ Project Setup

* Go to [start.spring.io](https://start.spring.io)
* Configure project:

  * Group: `ro.uaic.dbxdrgsl`
  * Artifact: `Lab03Homework`
  * Dependencies: **Spring Web**, **Spring Boot DevTools**, **Spring Boot Actuator**, **Spring AOP**, **Spring Context**, **Lombok** (optional)
* Extract and open in IntelliJ / VS Code.

### 2️⃣ Domain Model

```java
// Customer.java
public class Customer {
    private Long id;
    private String name;
    private boolean loyal;
    // constructor + getters
}

// Order.java
public class Order {
    private Long id;
    private Long customerId;
    private double totalAmount;
    // constructor + getters
}
```

### 3️⃣ Repository (In-memory)

```java
@Repository
public class CustomerRepository {
    private final Map<Long, Customer> customers = Map.of(
        1L, new Customer(1L, "Alice", true),
        2L, new Customer(2L, "Bob", false)
    );
    public Optional<Customer> findById(Long id) { return Optional.ofNullable(customers.get(id)); }
}
```

### 4️⃣ Discount Service Interface

```java
public interface DiscountService {
    double applyDiscount(Customer customer, Order order);
}
```

### 5️⃣ Implementations

#### Loyalty Discount

```java
@Service
@Profile("loyalty")
public class LoyaltyDiscountService implements DiscountService {
    public double applyDiscount(Customer c, Order o) {
        return o.getTotalAmount() * 0.1;
    }
}
```

#### Large Order Discount

```java
@Service
@Profile("largeorder")
public class LargeOrderDiscountService implements DiscountService {
    public double applyDiscount(Customer c, Order o) {
        return o.getTotalAmount() > 500 ? 50 : 0;
    }
}
```

#### No Discount

```java
@Service
@Profile("default")
public class NoDiscountService implements DiscountService {
    public double applyDiscount(Customer c, Order o) {
        return 0;
    }
}
```

### 6️⃣ Order Service

```java
@Service
public class OrderService {
    private final DiscountService discountService;
    private final CustomerRepository repository;
    private final ApplicationEventPublisher publisher;

    public OrderService(DiscountService discountService, CustomerRepository repository, ApplicationEventPublisher publisher) {
        this.discountService = discountService;
        this.repository = repository;
        this.publisher = publisher;
    }

    public double processOrder(Order order) {
        Customer customer = repository.findById(order.getCustomerId())
                .orElseThrow(() -> new RuntimeException("Customer not found"));

        double discount = discountService.applyDiscount(customer, order);
        System.out.printf("Applied by %s for %s: %.2f%n", discountService.getClass().getSimpleName(), customer.getName(), discount);

        if (discount > 40)
            publisher.publishEvent(new BigDiscountEvent(this, discount, customer.getName()));

        return discount;
    }
}
```

### 7️⃣ Custom Event + Listener

```java
public class BigDiscountEvent extends ApplicationEvent {
    private final double discount;
    private final String customerName;
    public BigDiscountEvent(Object src, double discount, String name) {
        super(src); this.discount = discount; this.customerName = name; }
}

@Component
public class BigDiscountListener {
    @EventListener
    public void onBigDiscount(BigDiscountEvent e) {
        System.out.printf("[EVENT] %.2f discount for %s%n", e.getDiscount(), e.getCustomerName());
    }
}
```

### 8️⃣ Aspect Validation

```java
@Aspect
@Component
public class DiscountValidationAspect {
    @Before("execution(* ro.uaic.dbxdrgsl.lab03homework.service.DiscountService.applyDiscount(..))")
    public void checkEligibility(JoinPoint jp) {
        Object[] args = jp.getArgs();
        Customer c = (Customer) args[0];
        Order o = (Order) args[1];
        if (!c.isLoyal() && o.getTotalAmount() < 100)
            throw new RuntimeException(c.getName() + " not eligible for discount");
        System.out.println("[ASPECT] Eligibility OK for " + c.getName());
    }
}
```

### 9️⃣ REST Controller

```java
@RestController
@RequestMapping("/orders")
public class OrderController {
    private final OrderService orderService;
    public OrderController(OrderService orderService) { this.orderService = orderService; }

    @GetMapping("/{id}")
    public String testOrder(@PathVariable Long id) {
        Order order = new Order(id, id, 600);
        return "Discount applied: " + orderService.processOrder(order);
    }
}
```

### 🔟 Configuration

`application.yml`

```yaml
spring:
  profiles:
    active: loyalty  # or largeorder or default
```

---

## 🧪 Run & Observe

```bash
mvn spring-boot:run -Dspring-boot.run.profiles=loyalty
```

Expected console:

```
[ASPECT] Eligibility OK for Alice
LoyaltyDiscountService applied: 60.0
Applied by LoyaltyDiscountService for Alice: 60.00
[EVENT] 60.00 discount for Alice
```

Test endpoint:

```
GET http://localhost:8080/orders/1
```

---

## 💡 Explanation Summary

| Component          | Responsibility                                 |
| ------------------ | ---------------------------------------------- |
| **IoC Container**  | Creates and injects dependencies automatically |
| **@Profile**       | Chooses which discount policy bean is active   |
| **@Aspect**        | Adds validation before discount logic runs     |
| **@EventListener** | Reacts to domain events asynchronously         |
| **OrderService**   | Business logic + event publishing              |
| **Loose Coupling** | Achieved via interfaces and DI                 |

---

## ✅ Learning Outcomes

* Demonstrated all **DI injection points** and **bean management**.
* Applied **cross-cutting concerns** via AOP.
* Used **Spring Events** for decoupled logic.
* Implemented **profile-based pluggable business rules**.

This project fulfills the *Compulsory (1p) + Homework (2p)* requirements of **Java Technologies – Lab 3 (2025‑2026)**.
