package uaic.dbxdrgsl.PrefSchedule.controller;

import uaic.dbxdrgsl.PrefSchedule.dto.LoginRequest;
import uaic.dbxdrgsl.PrefSchedule.dto.LoginResponse;
import uaic.dbxdrgsl.PrefSchedule.dto.RegisterRequest;
import uaic.dbxdrgsl.PrefSchedule.service.AuthService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @PostMapping("/login")
    public ResponseEntity<LoginResponse> login(@RequestBody LoginRequest loginRequest) {
        LoginResponse response = authService.login(loginRequest);
        return ResponseEntity.ok(response);
    }

    @PostMapping("/register")
    public ResponseEntity<String> register(@RequestBody RegisterRequest registerRequest) {
        authService.register(registerRequest);
        return ResponseEntity.ok("User registered successfully");
    }
}
