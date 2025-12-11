# Spring Security Implementation Summary

## ✅ All Requirements Completed

### Compulsory (1p) - COMPLETE
✅ Integrated Spring Security and protected all endpoints  
✅ Created `/api/auth/login` endpoint for authentication  
✅ Configured security chain to permit only `/api/auth/**` endpoints without authentication  
✅ All other endpoints require valid JWT token  

### Homework (2p) - COMPLETE

#### 1. Domain Refactoring ✅
- Refactored `Student` and `Instructor` to inherit from `User`
- Implemented `User` entity with authentication fields:
  - `username` (unique)
  - `password` (BCrypt hashed)
  - `email` (unique)
  - `firstName`, `lastName`
  - `role` (ADMIN, INSTRUCTOR, STUDENT)
  - `enabled` flag

#### 2. Database Configuration ✅
- Users and roles stored in database
- Spring Security reads users from database via `CustomUserDetailsService`
- Automatic user initialization with encrypted passwords

#### 3. JWT-Based Authentication ✅
- Implemented `JwtUtil` class for token generation and validation
- JWT tokens include user role information
- Token expiration: 24 hours (configurable)
- Signature: HMAC-SHA256

#### 4. Role-Based Access Control ✅
```
POST/PUT/DELETE endpoints protected by role:
- POST   /api/students       → ROLE_ADMIN
- PUT    /api/students/{id}  → ROLE_ADMIN, ROLE_INSTRUCTOR  
- DELETE /api/students/{id}  → ROLE_ADMIN
- GET    /api/students       → PUBLIC
```

#### 5. BCrypt Password Storage ✅
- All passwords encrypted with BCrypt (cost factor 10)
- Passwords never stored in plain text
- Registration endpoint hashes passwords automatically
- Authentication validates against hashed passwords

#### 6. User Registration Flow ✅
- `POST /api/auth/register` endpoint implemented
- Supports ADMIN, INSTRUCTOR, and STUDENT roles
- Validates unique username and email
- Creates associated `Student` or `Instructor` records
- Role-specific field validation

#### 7. Method-Level Security (@PreAuthorize) ✅
```java
@PreAuthorize("hasRole('ADMIN')")
@PostMapping
public ResponseEntity<Student> create(@RequestBody Student student)

@PreAuthorize("hasRole('ADMIN') or hasRole('INSTRUCTOR')")
@PutMapping("/{id}")
public ResponseEntity<Student> update(...)

@PreAuthorize("hasRole('ADMIN')")
@DeleteMapping("/{id}")
public ResponseEntity<Void> delete(...)
```

#### 8. Actuator Endpoint Security ✅
```yaml
Public endpoints:
- /actuator/health
- /actuator/info

Protected endpoints (require ADMIN):
- /actuator/metrics
- All other /actuator/** endpoints
```

## Project Structure

```
src/main/java/uaic/dbxdrgsl/PrefSchedule/
├── model/
│   ├── User.java                    # User entity with authentication
│   ├── UserRole.java                # ADMIN, INSTRUCTOR, STUDENT
│   ├── Student.java                 # 1-to-1 relationship with User
│   └── Instructor.java              # 1-to-1 relationship with User
├── dto/
│   ├── LoginRequest.java            # Login credentials
│   ├── LoginResponse.java           # JWT token response
│   └── RegisterRequest.java         # User registration data
├── controller/
│   ├── AuthController.java          # Login & Register endpoints
│   └── StudentController.java       # Protected CRUD with @PreAuthorize
├── service/
│   └── AuthService.java             # Authentication & Registration logic
├── repository/
│   └── UserRepository.java          # User data access
├── security/
│   ├── JwtUtil.java                 # JWT token generation/validation
│   ├── JwtAuthenticationFilter.java  # Extract & validate JWT
│   └── CustomUserDetailsService.java # Load users from database
└── config/
    └── SecurityConfig.java          # Spring Security configuration
```

## Key Files Changed

1. **pom.xml**
   - Added Spring Security dependency
   - Added JJWT 0.11.5 for JWT support
   - Added spring-security-test for testing

2. **src/main/resources/application.yaml**
   - Added JWT configuration (secret, expiration)
   - Configured Actuator endpoints
   - Configured H2 database

3. **Model Classes**
   - New: `User.java`, `UserRole.java`
   - Updated: `Student.java`, `Instructor.java` with User relationship

4. **New Security Classes**
   - `SecurityConfig.java` - Spring Security configuration
   - `JwtUtil.java` - JWT token operations
   - `JwtAuthenticationFilter.java` - JWT extraction & validation
   - `CustomUserDetailsService.java` - User loading from database

5. **New Authentication**
   - `AuthController.java` - Login & Register endpoints
   - `AuthService.java` - Authentication logic
   - DTOs: `LoginRequest.java`, `LoginResponse.java`, `RegisterRequest.java`

6. **Updated Components**
   - `StudentController.java` - Added @PreAuthorize annotations
   - `DataLoader.java` - Creates default users with encrypted passwords

## How to Run

### Build
```bash
mvn clean package
```

### Run Application
```bash
mvn spring-boot:run
```

The application starts on `http://localhost:8080`

### Test Login
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

### Use Token
```bash
curl -X GET http://localhost:8080/api/students \
  -H "Authorization: Bearer <token>"
```

## Default Test Users

| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | ADMIN |
| john_inst0 | password123 | INSTRUCTOR |
| mary_std0 | password123 | STUDENT |

## Security Features Summary

✅ Spring Security fully integrated
✅ JWT-based stateless authentication
✅ Role-based access control (RBAC)
✅ BCrypt password encryption
✅ Method-level security with @PreAuthorize
✅ User registration with validation
✅ Database-driven user management
✅ Actuator endpoint security
✅ CSRF protection enabled
✅ Stateless session management

## Testing

Run security integration tests:
```bash
mvn test
```

Tests verify:
- Login with valid/invalid credentials
- User registration
- Public endpoint access
- Protected endpoint access control
- Token validation
- Role-based authorization
- Actuator endpoint security

## Notes

- All passwords are automatically hashed with BCrypt
- JWT tokens expire after 24 hours
- Students and Instructors are now part of the User management system
- All endpoints except `/api/auth/**` require authentication
- Only ADMIN users can access `/actuator/metrics`
