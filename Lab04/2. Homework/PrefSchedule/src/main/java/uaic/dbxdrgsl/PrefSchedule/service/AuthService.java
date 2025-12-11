package uaic.dbxdrgsl.PrefSchedule.service;

import uaic.dbxdrgsl.PrefSchedule.model.*;
import uaic.dbxdrgsl.PrefSchedule.dto.LoginRequest;
import uaic.dbxdrgsl.PrefSchedule.dto.LoginResponse;
import uaic.dbxdrgsl.PrefSchedule.dto.RegisterRequest;
import uaic.dbxdrgsl.PrefSchedule.repository.InstructorRepository;
import uaic.dbxdrgsl.PrefSchedule.repository.StudentRepository;
import uaic.dbxdrgsl.PrefSchedule.repository.UserRepository;
import uaic.dbxdrgsl.PrefSchedule.security.JwtUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class AuthService {

    @Autowired
    private UserRepository userRepository;

    @Autowired
    private StudentRepository studentRepository;

    @Autowired
    private InstructorRepository instructorRepository;

    @Autowired
    private AuthenticationManager authenticationManager;

    @Autowired
    private PasswordEncoder passwordEncoder;

    @Autowired
    private JwtUtil jwtUtil;

    @Autowired
    private UserDetailsService userDetailsService;

    @Transactional
    public LoginResponse login(LoginRequest loginRequest) {
        Authentication authentication = authenticationManager.authenticate(
            new UsernamePasswordAuthenticationToken(
                loginRequest.getUsername(),
                loginRequest.getPassword()
            )
        );

        UserDetails userDetails = userDetailsService.loadUserByUsername(loginRequest.getUsername());
        String token = jwtUtil.generateToken(userDetails);
        User user = userRepository.findByUsername(loginRequest.getUsername()).orElseThrow();

        return LoginResponse.builder()
            .token(token)
            .username(user.getUsername())
            .role(user.getRole().name())
            .userId(user.getId())
            .build();
    }

    @Transactional
    public void register(RegisterRequest registerRequest) {
        if (userRepository.existsByUsername(registerRequest.getUsername())) {
            throw new RuntimeException("Username already exists");
        }
        if (userRepository.existsByEmail(registerRequest.getEmail())) {
            throw new RuntimeException("Email already exists");
        }

        User user = User.builder()
            .username(registerRequest.getUsername())
            .password(passwordEncoder.encode(registerRequest.getPassword()))
            .email(registerRequest.getEmail())
            .firstName(registerRequest.getFirstName())
            .lastName(registerRequest.getLastName())
            .role(registerRequest.getRole())
            .enabled(true)
            .build();

        user = userRepository.save(user);

        if (registerRequest.getRole() == UserRole.STUDENT) {
            if (registerRequest.getStudentNumber() == null || registerRequest.getStudentNumber().isEmpty()) {
                throw new RuntimeException("Student number is required for STUDENT role");
            }
            studentRepository.save(Student.builder()
                .user(user)
                .studentNumber(registerRequest.getStudentNumber())
                .group(registerRequest.getGroup())
                .build());
        } else if (registerRequest.getRole() == UserRole.INSTRUCTOR) {
            if (registerRequest.getDepartment() == null || registerRequest.getDepartment().isEmpty()) {
                throw new RuntimeException("Department is required for INSTRUCTOR role");
            }
            instructorRepository.save(Instructor.builder()
                .user(user)
                .department(registerRequest.getDepartment())
                .specialization(registerRequest.getSpecialization())
                .build());
        }
    }
}
