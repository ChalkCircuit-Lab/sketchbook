# PrefSchedule - Spring Security Implementation Guide

## Overview
This project implements comprehensive Spring Security with JWT authentication, role-based access control, and BCrypt password encryption for a course preference scheduling system.

## Features Implemented

### 1. **Spring Security Integration** ✅
- Configured `SecurityConfig` with CSRF disabled and stateless session management
- Protected all endpoints except `/api/auth/**`, `/actuator/health`, and `/actuator/info`
- GET endpoints are publicly accessible
- POST/PUT/DELETE endpoints require authentication and role-based authorization

### 2. **User Domain Refactoring** ✅
- Created `User` entity as base class for all users in the system
- `Student` and `Instructor` now have a 1-to-1 relationship with `User`
- Implemented `UserRole` enum with three roles: ADMIN, INSTRUCTOR, STUDENT
- Each user has authentication credentials (username, password, email)

### 3. **JWT-Based Authentication** ✅
- Created `JwtUtil` class for token generation and validation
- Tokens include claims with user role information
- Token expiration configurable via `jwt.expiration` property (default: 24 hours)
- Secret key configurable via `jwt.secret` property

### 4. **Authentication Endpoints** ✅
- **POST `/api/auth/login`**: Authenticate user and receive JWT token
  ```bash
  curl -X POST http://localhost:8080/api/auth/login \
    -H "Content-Type: application/json" \
    -d '{"username":"admin","password":"admin123"}'
  ```
  
- **POST `/api/auth/register`**: Register new user (STUDENT, INSTRUCTOR, or ADMIN)
  ```bash
  curl -X POST http://localhost:8080/api/auth/register \
    -H "Content-Type: application/json" \
    -d '{
      "username":"john_student",
      "password":"password123",
      "email":"john@student.uni.edu",
      "firstName":"John",
      "lastName":"Doe",
      "role":"STUDENT",
      "studentNumber":"STU001",
      "group":"Group1"
    }'
  ```

### 5. **Role-Based Access Control** ✅
- **ADMIN**: Can perform all operations (POST/PUT/DELETE on all resources)
- **INSTRUCTOR**: Can update student records and manage courses
- **STUDENT**: Can only view courses and enroll in courses
- Implemented via `@PreAuthorize` annotations on controller methods

### 6. **Method-Level Security** ✅
- `@PreAuthorize` annotations on sensitive endpoints:
  - `POST /api/students` → requires `ROLE_ADMIN`
  - `PUT /api/students/{id}` → requires `ROLE_ADMIN` or `ROLE_INSTRUCTOR`
  - `DELETE /api/students/{id}` → requires `ROLE_ADMIN`
- `GET` endpoints are public and accessible without authentication

### 7. **BCrypt Password Encryption** ✅
- Configured `BCryptPasswordEncoder` as the password encoder
- All passwords stored as BCrypt hashes in the database
- Registration endpoint automatically hashes passwords before saving
- Passwords validated during authentication process

### 8. **User Registration Flow** ✅
- Validates unique username and email
- Supports different user types (STUDENT, INSTRUCTOR, ADMIN)
- Creates associated `Student` or `Instructor` record based on role
- Validates role-specific fields:
  - STUDENT: requires `studentNumber`
  - INSTRUCTOR: requires `department`

### 9. **Actuator Endpoint Security** ✅
```yaml
# Public endpoints
- /actuator/health
- /actuator/info

# Protected endpoints (require ADMIN role)
- /actuator/metrics
- All other /actuator/** endpoints
```

## Default Users

The application automatically initializes with these test users:

| Username | Password | Role | Email |
|----------|----------|------|-------|
| admin | admin123 | ADMIN | admin@uni.edu |
| john_inst0 | password123 | INSTRUCTOR | john_inst0@uni.edu |
| mary_std0 | password123 | STUDENT | mary_std0@student.uni.edu |

## How to Use

### 1. Build the Project
```bash
mvn clean install
```

### 2. Run the Application
```bash
mvn spring-boot:run
```

The application will start on `http://localhost:8080`

### 3. Login and Get JWT Token
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

Response:
```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "username": "admin",
  "role": "ADMIN",
  "userId": 1
}
```

### 4. Use Token to Access Protected Endpoints
```bash
curl -X GET http://localhost:8080/api/students \
  -H "Authorization: Bearer <token>"
```

### 5. Test Role-Based Access
```bash
# This will fail (requires ADMIN role)
curl -X POST http://localhost:8080/api/students \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <student_token>" \
  -d '{"studentNumber":"STU123","group":"Group1"}'
```

## Configuration Properties

```yaml
# JWT Configuration
jwt:
  secret: mySecretKeyForJWTTokenGenerationAndValidationPurposePrefSchedule12345
  expiration: 86400000  # 24 hours in milliseconds

# Actuator Configuration
management:
  endpoints:
    web:
      exposure:
        include: health,info,metrics
  endpoint:
    health:
      show-details: when-authorized
```

## Project Structure

```
src/main/java/uaic/dbxdrgsl/PrefSchedule/
├── model/
│   ├── User.java              # User entity with authentication
│   ├── UserRole.java          # Enum: ADMIN, INSTRUCTOR, STUDENT
│   ├── Student.java           # Extends User relationship
│   └── Instructor.java        # Extends User relationship
├── controller/
│   ├── AuthController.java    # Login & Register endpoints
│   └── StudentController.java # Protected CRUD operations
├── service/
│   └── AuthService.java       # Authentication logic
├── repository/
│   └── UserRepository.java    # User data access
├── security/
│   ├── JwtUtil.java           # JWT token generation & validation
│   ├── JwtAuthenticationFilter.java  # Filter for token extraction
│   └── CustomUserDetailsService.java # Load user from database
└── config/
    └── SecurityConfig.java    # Spring Security configuration
```

## Testing

Run the security integration tests:
```bash
mvn test
```

### Key Test Cases
- ✅ Login with valid credentials
- ✅ Login with invalid credentials (should fail)
- ✅ Register new student
- ✅ Public GET endpoints accessible without auth
- ✅ Protected POST endpoints require authentication
- ✅ Actuator health endpoint public
- ✅ Actuator metrics endpoint protected

## Security Best Practices Implemented

1. **Password Security**
   - Passwords hashed with BCrypt (cost factor 10)
   - Never stored in plain text
   - Validated during authentication

2. **Token Security**
   - JWT tokens signed with HMAC-SHA256
   - Configurable expiration time
   - Verified on every request

3. **Access Control**
   - Role-based authorization via Spring Security
   - Method-level security with @PreAuthorize
   - Stateless session management

4. **API Security**
   - CSRF protection configured
   - Unauthorized access returns 401 Unauthorized
   - Forbidden access returns 403 Forbidden

## Endpoints Summary

| Method | Endpoint | Public | Roles | Description |
|--------|----------|--------|-------|-------------|
| POST | /api/auth/login | ✅ | - | Login and receive JWT |
| POST | /api/auth/register | ✅ | - | Register new user |
| GET | /api/students | ✅ | - | List all students |
| GET | /api/students/{id} | ✅ | - | Get student details |
| POST | /api/students | ❌ | ADMIN | Create new student |
| PUT | /api/students/{id} | ❌ | ADMIN, INSTRUCTOR | Update student |
| DELETE | /api/students/{id} | ❌ | ADMIN | Delete student |
| GET | /actuator/health | ✅ | - | Health check |
| GET | /actuator/info | ✅ | - | Application info |
| GET | /actuator/metrics | ❌ | ADMIN | Application metrics |

## Troubleshooting

### Token Expired Error
- Token expires after 24 hours by default
- Adjust `jwt.expiration` property to change duration

### Authorization Failed
- Check that user role matches required role
- Verify token is passed in `Authorization: Bearer <token>` header

### User Registration Fails
- Ensure unique username and email
- Provide role-specific fields (studentNumber for STUDENT, department for INSTRUCTOR)

## Future Enhancements

- [ ] Refresh token mechanism
- [ ] Token blacklisting for logout
- [ ] Multi-factor authentication (MFA)
- [ ] OAuth2 integration
- [ ] Rate limiting for login attempts
- [ ] Account lockout after failed attempts
- [ ] Email verification for registration
- [ ] Password reset functionality
