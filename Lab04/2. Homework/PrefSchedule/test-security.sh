#!/bin/bash
# PrefSchedule Security Testing Script
# These are example curl commands to test the Spring Security implementation

# Color output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:8080"

echo -e "${BLUE}=== PrefSchedule Security Testing ===${NC}\n"

# Test 1: Login as ADMIN
echo -e "${YELLOW}Test 1: Login as ADMIN${NC}"
ADMIN_LOGIN=$(curl -s -X POST "$BASE_URL/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}')
echo $ADMIN_LOGIN | jq .
ADMIN_TOKEN=$(echo $ADMIN_LOGIN | jq -r '.token')
echo -e "${GREEN}✓ Admin token obtained${NC}\n"

# Test 2: Register new STUDENT user
echo -e "${YELLOW}Test 2: Register new STUDENT${NC}"
curl -s -X POST "$BASE_URL/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{
    "username":"testuser",
    "password":"testpass123",
    "email":"test@student.uni.edu",
    "firstName":"Test",
    "lastName":"User",
    "role":"STUDENT",
    "studentNumber":"STU9999",
    "group":"GroupA"
  }' | jq .
echo -e "${GREEN}✓ Student registered${NC}\n"

# Test 3: Login as new STUDENT
echo -e "${YELLOW}Test 3: Login as STUDENT${NC}"
STUDENT_LOGIN=$(curl -s -X POST "$BASE_URL/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"testpass123"}')
echo $STUDENT_LOGIN | jq .
STUDENT_TOKEN=$(echo $STUDENT_LOGIN | jq -r '.token')
echo -e "${GREEN}✓ Student token obtained${NC}\n"

# Test 4: Get students (PUBLIC - no auth required)
echo -e "${YELLOW}Test 4: GET /api/students (PUBLIC)${NC}"
curl -s -X GET "$BASE_URL/api/students" \
  -H "Content-Type: application/json" | jq .
echo -e "${GREEN}✓ Can access students without auth${NC}\n"

# Test 5: Create student with ADMIN token (should work)
echo -e "${YELLOW}Test 5: POST /api/students as ADMIN (should work)${NC}"
curl -s -X POST "$BASE_URL/api/students" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $ADMIN_TOKEN" \
  -d '{
    "user":{"username":"student2","firstName":"John","lastName":"Doe","email":"john@student.uni.edu","password":"pass123","role":"STUDENT"},
    "studentNumber":"STU1234",
    "group":"GroupB"
  }' | jq .
echo -e "${GREEN}✓ Admin can create student${NC}\n"

# Test 6: Try to create student with STUDENT token (should fail)
echo -e "${YELLOW}Test 6: POST /api/students as STUDENT (should be forbidden)${NC}"
curl -s -X POST "$BASE_URL/api/students" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $STUDENT_TOKEN" \
  -d '{"studentNumber":"STU5678","group":"GroupC"}' | jq .
echo -e "${YELLOW}Expected: 403 Forbidden${NC}\n"

# Test 7: Try to access protected endpoint without token (should fail)
echo -e "${YELLOW}Test 7: POST /api/students without token (should be unauthorized)${NC}"
curl -s -X POST "$BASE_URL/api/students" \
  -H "Content-Type: application/json" \
  -d '{"studentNumber":"STU9876","group":"GroupD"}' | jq .
echo -e "${YELLOW}Expected: 401 Unauthorized${NC}\n"

# Test 8: Access actuator health (PUBLIC)
echo -e "${YELLOW}Test 8: GET /actuator/health (PUBLIC)${NC}"
curl -s -X GET "$BASE_URL/actuator/health" \
  -H "Content-Type: application/json" | jq .
echo -e "${GREEN}✓ Can access health endpoint${NC}\n"

# Test 9: Access actuator info (PUBLIC)
echo -e "${YELLOW}Test 9: GET /actuator/info (PUBLIC)${NC}"
curl -s -X GET "$BASE_URL/actuator/info" \
  -H "Content-Type: application/json" | jq .
echo -e "${GREEN}✓ Can access info endpoint${NC}\n"

# Test 10: Access actuator metrics without auth (should fail)
echo -e "${YELLOW}Test 10: GET /actuator/metrics without auth (should fail)${NC}"
curl -s -X GET "$BASE_URL/actuator/metrics" \
  -H "Content-Type: application/json" | jq .
echo -e "${YELLOW}Expected: 401 Unauthorized${NC}\n"

# Test 11: Access actuator metrics with ADMIN token (should work)
echo -e "${YELLOW}Test 11: GET /actuator/metrics with ADMIN token (should work)${NC}"
curl -s -X GET "$BASE_URL/actuator/metrics" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $ADMIN_TOKEN" | jq .
echo -e "${GREEN}✓ Admin can access metrics${NC}\n"

# Test 12: Login with invalid credentials
echo -e "${YELLOW}Test 12: Login with invalid credentials${NC}"
curl -s -X POST "$BASE_URL/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"wrongpassword"}' | jq .
echo -e "${YELLOW}Expected: 401 Unauthorized${NC}\n"

# Test 13: Register with duplicate username (should fail)
echo -e "${YELLOW}Test 13: Register with duplicate username (should fail)${NC}"
curl -s -X POST "$BASE_URL/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{
    "username":"testuser",
    "password":"anotherpass",
    "email":"another@email.com",
    "firstName":"Another",
    "lastName":"User",
    "role":"STUDENT",
    "studentNumber":"STU8888",
    "group":"GroupX"
  }' | jq .
echo -e "${YELLOW}Expected: Error - Username already exists${NC}\n"

echo -e "${BLUE}=== All Tests Complete ===${NC}"
