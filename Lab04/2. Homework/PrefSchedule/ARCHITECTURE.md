# Class and Architecture Reference

## Entity Model Diagram

```
┌─────────────────────────────────────────┐
│           User Entity                   │
├─────────────────────────────────────────┤
│ - id: Long (PK)                         │
│ - username: String (UNIQUE)             │
│ - password: String (BCrypt)             │
│ - email: String (UNIQUE)                │
│ - firstName: String                     │
│ - lastName: String                      │
│ - role: UserRole (ADMIN/INSTRUCTOR/...)│
│ - enabled: Boolean                      │
│ - createdAt: LocalDateTime              │
│ - updatedAt: LocalDateTime              │
└─────────────────────────────────────────┘
         ▲              ▲
         │              │
      1-to-1          1-to-1
    OneToOne        OneToOne
         │              │
    ┌────┴────┐    ┌────┴─────────┐
    │ Student │    │ Instructor   │
    ├─────────┤    ├──────────────┤
    │ - id    │    │ - id         │
    │ - user  │    │ - user       │
    │ - num   │    │ - dept       │
    │ - group │    │ - spec       │
    │ - enrol │    │ - courses    │
    └─────────┘    └──────────────┘
```

## Authentication Flow

```
┌─────────────────────────────────────────────────────────────┐
│ Client Request                                              │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
        ┌──────────────────────────────────────┐
        │ JwtAuthenticationFilter              │
        │ - Extracts JWT from Authorization   │
        │ - Validates token                   │
        │ - Sets SecurityContext              │
        └──────────────┬───────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────────────┐
        │ JwtUtil                              │
        │ - extractUsername()                 │
        │ - validateToken()                   │
        │ - Verifies signature & expiration   │
        └──────────────┬───────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────────────┐
        │ CustomUserDetailsService            │
        │ - loadUserByUsername()              │
        │ - Loads from UserRepository         │
        │ - Creates UserDetails               │
        └──────────────┬───────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────────────┐
        │ UserRepository                       │
        │ - Queries User from database        │
        │ - Converts to UserDetails           │
        └──────────────┬───────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────────────┐
        │ SecurityContext                      │
        │ - Stores authenticated principal    │
        │ - Authorities loaded from User.role │
        └──────────────┬───────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────────────┐
        │ Controller Method                    │
        │ @PreAuthorize checks authority      │
        │ Processes request                   │
        └──────────────────────────────────────┘
```

## Class Hierarchy

### Model Classes
```
User (JPA Entity)
  └── Relationships:
      ├── OneToOne → Student
      └── OneToOne → Instructor

Student (JPA Entity)
  ├── Fields: id, user, studentNumber, group, enrollments
  └── Methods: addEnrollment(), removeEnrollment()

Instructor (JPA Entity)
  ├── Fields: id, user, department, specialization, courses
  └── Methods: addCourse(), removeCourse()

UserRole (Enum)
  ├── ADMIN
  ├── INSTRUCTOR
  └── STUDENT
```

### Service Layer
```
AuthService
  ├── login(LoginRequest): LoginResponse
  ├── register(RegisterRequest): void
  └── Dependencies:
      ├── UserRepository
      ├── StudentRepository
      ├── InstructorRepository
      ├── AuthenticationManager
      ├── PasswordEncoder
      ├── JwtUtil
      └── UserDetailsService
```

### Controller Layer
```
AuthController
  ├── @PostMapping("/login")
  │   └── login(LoginRequest): ResponseEntity<LoginResponse>
  ├── @PostMapping("/register")
  │   └── register(RegisterRequest): ResponseEntity<String>
  └── Dependencies: AuthService

StudentController
  ├── @GetMapping ← Public
  ├── @GetMapping("/{id}") ← Public
  ├── @PostMapping @PreAuthorize("ROLE_ADMIN")
  ├── @PutMapping @PreAuthorize("ROLE_ADMIN || ROLE_INSTRUCTOR")
  └── @DeleteMapping @PreAuthorize("ROLE_ADMIN")
```

### Security Layer
```
JwtUtil
  ├── generateToken(UserDetails): String
  ├── extractUsername(String): String
  ├── extractExpiration(String): Date
  ├── validateToken(String, UserDetails): Boolean
  ├── extractAllClaims(String): Claims
  ├── isTokenExpired(String): Boolean
  └── getSigningKey(): SecretKey

JwtAuthenticationFilter extends OncePerRequestFilter
  ├── doFilterInternal(...)
  ├── extractJwtFromRequest(HttpServletRequest): String
  └── Dependencies:
      ├── JwtUtil
      └── UserDetailsService

CustomUserDetailsService implements UserDetailsService
  ├── loadUserByUsername(String): UserDetails
  └── Dependencies: UserRepository

SecurityConfig
  ├── passwordEncoder(): PasswordEncoder
  ├── authenticationManager(...): AuthenticationManager
  ├── filterChain(HttpSecurity): SecurityFilterChain
  └── Configuration:
      ├── CSRF disabled
      ├── Stateless sessions
      ├── Permit /api/auth/**
      ├── Permit GET /api/**
      ├── Require auth for POST/PUT/DELETE
      └── Actuator protection
```

### Repository Layer
```
UserRepository extends JpaRepository<User, Long>
  ├── findByUsername(String): Optional<User>
  ├── findByEmail(String): Optional<User>
  ├── existsByUsername(String): boolean
  └── existsByEmail(String): boolean

StudentRepository extends JpaRepository<Student, Long>
  └── findByStudentNumber(String): Optional<Student>

InstructorRepository extends JpaRepository<Instructor, Long>
```

### DTO Classes
```
LoginRequest
  ├── username: String
  └── password: String

LoginResponse
  ├── token: String
  ├── username: String
  ├── role: String
  └── userId: Long

RegisterRequest
  ├── username: String
  ├── password: String
  ├── email: String
  ├── firstName: String
  ├── lastName: String
  ├── role: UserRole
  ├── studentNumber: String (optional)
  ├── group: String (optional)
  ├── department: String (optional)
  └── specialization: String (optional)
```

## Security Chain Configuration

```
HTTP Request
    │
    ├─→ CsrfFilter (disabled)
    ├─→ SecurityContextPersistenceFilter
    ├─→ JwtAuthenticationFilter ← Custom
    │   └─→ Extract & Validate JWT
    ├─→ AuthorizationFilter
    │   ├─→ /api/auth/** → PERMIT
    │   ├─→ /actuator/health → PERMIT
    │   ├─→ /actuator/info → PERMIT
    │   ├─→ GET /api/** → PERMIT
    │   ├─→ /actuator/** → REQUIRE ADMIN
    │   └─→ POST/PUT/DELETE /api/** → REQUIRE AUTH
    │
    └─→ Controller
```

## Request/Response Cycle

### Login Request/Response
```
POST /api/auth/login
Content-Type: application/json

{
  "username": "admin",
  "password": "admin123"
}

↓ (ProcessedBy: AuthController → AuthService → AuthenticationManager)

200 OK
Content-Type: application/json

{
  "token": "eyJhbGciOiJIUzI1NiJ9...",
  "username": "admin",
  "role": "ADMIN",
  "userId": 1
}
```

### Protected Request
```
GET /api/students
Authorization: Bearer <token>

↓ (ProcessedBy: JwtAuthenticationFilter → JwtUtil → Controller)

200 OK
Content-Type: application/json

[...]
```

### Unauthorized Request
```
GET /api/students

↓ (No token provided)

401 Unauthorized
```

### Forbidden Request
```
POST /api/students
Authorization: Bearer <student_token>

↓ (@PreAuthorize("hasRole('ADMIN')") fails)

403 Forbidden
```

## Configuration Properties

```yaml
jwt:
  secret: "..."
  expiration: 86400000

server:
  servlet:
    context-path: /

management:
  endpoints:
    web:
      exposure:
        include: health,info,metrics

spring:
  jpa:
    hibernate:
      ddl-auto: update
  datasource:
    url: jdbc:h2:mem:prefdb
```

## Dependencies

```xml
<!-- Core -->
spring-boot-starter-web
spring-boot-starter-data-jpa
spring-boot-starter-security

<!-- JWT -->
jjwt-api:0.11.5
jjwt-impl:0.11.5
jjwt-jackson:0.11.5

<!-- Database -->
h2database
postgresql

<!-- Tools -->
lombok
spring-boot-starter-actuator
```

## Key Design Patterns

1. **Filter Pattern** - JwtAuthenticationFilter
2. **Service Layer** - AuthService
3. **Repository Pattern** - UserRepository
4. **DTO Pattern** - LoginRequest/Response
5. **Builder Pattern** - User, Student, Instructor
6. **Singleton** - SecurityConfig, JwtUtil
7. **Strategy Pattern** - PasswordEncoder

## Security Annotations Used

```java
@EnableWebSecurity              // Enable Spring Security
@EnableMethodSecurity          // Enable @PreAuthorize
@PreAuthorize("hasRole('...')")  // Method-level security
@Component                     // Register beans
@Configuration                 // Configuration class
@Bean                         // Define beans
@Entity                       // JPA entity
@Repository                   // Repository bean
@Service                      // Service bean
@RestController              // REST controller
@RequestMapping              // Route mapping
@PostMapping/@GetMapping      // HTTP method mapping
```

## Token Structure

```
Header.Payload.Signature

Header:
{
  "alg": "HS256",
  "typ": "JWT"
}

Payload:
{
  "sub": "admin",
  "role": ["ROLE_ADMIN"],
  "iat": 1702253200,
  "exp": 1702339600
}

Signature:
HMACSHA256(base64UrlEncode(header) + "." + base64UrlEncode(payload), secret)
```

This architecture provides a clean, scalable, and secure implementation of JWT-based authentication with role-based access control for the PrefSchedule application.
