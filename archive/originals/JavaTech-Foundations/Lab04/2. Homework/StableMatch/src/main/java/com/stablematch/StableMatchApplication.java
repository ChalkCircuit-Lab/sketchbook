package com.stablematch;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

/**
 * Main Spring Boot Application class for StableMatch service.
 * Dedicated to solving stable matching problems for student course assignment.
 */
@SpringBootApplication
public class StableMatchApplication {

    public static void main(String[] args) {
        SpringApplication.run(StableMatchApplication.class, args);
    }
}
