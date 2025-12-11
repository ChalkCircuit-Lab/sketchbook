package uaic.dbxdrgsl.PrefSchedule;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.MvcResult;
import uaic.dbxdrgsl.PrefSchedule.dto.LoginRequest;
import uaic.dbxdrgsl.PrefSchedule.dto.LoginResponse;
import uaic.dbxdrgsl.PrefSchedule.dto.RegisterRequest;
import uaic.dbxdrgsl.PrefSchedule.model.UserRole;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
public class SecurityIntegrationTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private ObjectMapper objectMapper;

    @Test
    public void testLoginWithValidCredentials() throws Exception {
        LoginRequest loginRequest = new LoginRequest("admin", "admin123");

        MvcResult result = mockMvc.perform(post("/api/auth/login")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(loginRequest)))
            .andExpect(status().isOk())
            .andExpect(jsonPath("$.token").isNotEmpty())
            .andExpect(jsonPath("$.username").value("admin"))
            .andExpect(jsonPath("$.role").value("ADMIN"))
            .andReturn();

        System.out.println("Login Response: " + result.getResponse().getContentAsString());
    }

    @Test
    public void testLoginWithInvalidCredentials() throws Exception {
        LoginRequest loginRequest = new LoginRequest("admin", "wrongpassword");

        mockMvc.perform(post("/api/auth/login")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(loginRequest)))
            .andExpect(status().isUnauthorized());
    }

    @Test
    public void testRegisterNewStudent() throws Exception {
        RegisterRequest registerRequest = new RegisterRequest();
        registerRequest.setUsername("teststudent");
        registerRequest.setPassword("password123");
        registerRequest.setEmail("test@student.uni.edu");
        registerRequest.setFirstName("Test");
        registerRequest.setLastName("Student");
        registerRequest.setRole(UserRole.STUDENT);
        registerRequest.setStudentNumber("STU99999");
        registerRequest.setGroup("Group1");

        mockMvc.perform(post("/api/auth/register")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(registerRequest)))
            .andExpect(status().isOk());
    }

    @Test
    public void testPublicGetStudentsEndpoint() throws Exception {
        mockMvc.perform(get("/api/students")
                .contentType(MediaType.APPLICATION_JSON))
            .andExpect(status().isOk());
    }

    @Test
    public void testProtectedPostStudentEndpointWithoutAuth() throws Exception {
        String studentJson = "{\"id\":null,\"user\":{\"id\":null,\"username\":\"test\",\"firstName\":\"Test\",\"lastName\":\"Student\"},\"studentNumber\":\"STU12345\",\"group\":\"Group1\"}";

        mockMvc.perform(post("/api/students")
                .contentType(MediaType.APPLICATION_JSON)
                .content(studentJson))
            .andExpect(status().isUnauthorized());
    }

    @Test
    public void testActuatorHealthPublic() throws Exception {
        mockMvc.perform(get("/actuator/health")
                .contentType(MediaType.APPLICATION_JSON))
            .andExpect(status().isOk());
    }

    @Test
    public void testActuatorMetricsProtected() throws Exception {
        mockMvc.perform(get("/actuator/metrics")
                .contentType(MediaType.APPLICATION_JSON))
            .andExpect(status().isUnauthorized());
    }

    @Test
    public void testAuthorizationFlow() throws Exception {
        // First, login to get a token
        LoginRequest loginRequest = new LoginRequest("admin", "admin123");

        MvcResult loginResult = mockMvc.perform(post("/api/auth/login")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(loginRequest)))
            .andExpect(status().isOk())
            .andReturn();

        String responseBody = loginResult.getResponse().getContentAsString();
        LoginResponse loginResponse = objectMapper.readValue(responseBody, LoginResponse.class);
        String token = loginResponse.getToken();

        // Now use the token to access protected endpoint
        String studentJson = "{\"id\":null,\"user\":{\"id\":null,\"username\":\"authorized\",\"firstName\":\"Auth\",\"lastName\":\"User\"},\"studentNumber\":\"STU11111\",\"group\":\"Group1\"}";

        mockMvc.perform(post("/api/students")
                .contentType(MediaType.APPLICATION_JSON)
                .header("Authorization", "Bearer " + token)
                .content(studentJson))
            .andExpect(status().isCreated());
    }
}
