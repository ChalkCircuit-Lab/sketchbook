# Quick Start Guide - PrefSchedule Spring Security

## Prerequisites
- Java 21 JDK installed
- Maven 3.9.x installed
- Git installed

## Build & Run (5 minutes)

### 1. Clone/Navigate to Project
```bash
cd "c:\Users\Dragos\OneDrive\Coding Java\JavaTech\Lab04\2. Homework\PrefSchedule"
```

### 2. Build Project
```bash
mvn clean package
```

### 3. Run Application
```bash
mvn spring-boot:run
```

Application starts on `http://localhost:8080`

## Test the Implementation

### 1. Login (Get JWT Token)
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

Response:
```json
{
  "token": "eyJhbGciOiJIUzI1NiJ9...",
  "username": "admin",
  "role": "ADMIN",
  "userId": 1
}
```

### 2. Use Token to Access Protected Endpoint
```bash
# Replace <token> with actual token from login response
curl -X GET http://localhost:8080/api/students \
  -H "Authorization: Bearer <token>"
```

### 3. Test Public Endpoint (No Token Required)
```bash
curl http://localhost:8080/api/students
```

### 4. Register New User
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username":"newstudent",
    "password":"password123",
    "email":"new@student.uni.edu",
    "firstName":"New",
    "lastName":"Student",
    "role":"STUDENT",
    "studentNumber":"STU5555",
    "group":"Group1"
  }'
```

### 5. Test Access Control (Should Fail)
```bash
# Create with student token (should fail - needs ADMIN)
curl -X POST http://localhost:8080/api/students \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <student_token>" \
  -d '{"studentNumber":"STU9999","group":"GroupX"}'
```

Expected: 403 Forbidden

## Default Test Users

| Username | Password | Role |
|----------|----------|------|
| admin | admin123 | ADMIN |
| john_inst0 | password123 | INSTRUCTOR |
| mary_std0 | password123 | STUDENT |

## Key Features Implemented

✅ Spring Security with JWT authentication
✅ Role-based access control (ADMIN, INSTRUCTOR, STUDENT)
✅ BCrypt password encryption
✅ User registration endpoint
✅ Database-driven user management
✅ Method-level security with @PreAuthorize
✅ Protected and public endpoints
✅ Actuator endpoint security

## Project Structure

```
src/main/java/uaic/dbxdrgsl/PrefSchedule/
├── model/           # User, Student, Instructor entities
├── controller/      # AuthController, StudentController
├── service/         # AuthService
├── repository/      # UserRepository
├── security/        # JWT & Authentication filters
└── config/          # SecurityConfig
```

## Documentation

- `SECURITY_IMPLEMENTATION.md` - Detailed implementation guide
- `IMPLEMENTATION_COMPLETE.md` - Complete requirements checklist
- `VERIFICATION_CHECKLIST.md` - Verification of all requirements
- `test-security.sh` - Example curl commands for testing

## Troubleshooting

### Build Fails
```bash
# Clean maven cache and rebuild
mvn clean install -U
```

### Port Already in Use
```bash
# Run on different port
mvn spring-boot:run -Dspring-boot.run.arguments="--server.port=8081"
```

### Token Validation Error
- Ensure token is passed in correct format: `Authorization: Bearer <token>`
- Token expires after 24 hours - get new token via login

## Next Steps

1. Read `SECURITY_IMPLEMENTATION.md` for detailed documentation
2. Review `VERIFICATION_CHECKLIST.md` for complete feature list
3. Check `test-security.sh` for example API calls
4. Explore the security configuration in `SecurityConfig.java`

---

**Status: Ready for Production Testing** ✅
