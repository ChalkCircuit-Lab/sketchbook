package com.stablematch.controller;

import com.stablematch.algorithm.StableMatchingService;
import com.stablematch.dto.StableMatchingRequestDTO;
import com.stablematch.dto.StableMatchingResponseDTO;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.stream.Collectors;

/**
 * REST Controller for the Stable Matching Algorithm API.
 * Handles HTTP requests for solving student-to-course assignment problems.
 */
@Slf4j
@RestController
@RequestMapping("/v1/matching")
@RequiredArgsConstructor
@CrossOrigin(origins = "*", maxAge = 3600)
public class StableMatchingController {

    private final StableMatchingService stableMatchingService;

    // Store last solved matching result for querying (in production, use a database)
    private StableMatchingResponseDTO lastResult;

    /**
     * Solves a stable matching problem for student-to-course assignment.
     *
     * @param request the stable matching problem containing student and course preferences
     * @return ResponseEntity containing the assignment result
     */
    @PostMapping("/solve")
    public ResponseEntity<StableMatchingResponseDTO> solveMatching(
            @RequestBody StableMatchingRequestDTO request) {

        log.info("Received stable matching request with {} students and {} courses",
                request.getStudents().size(), request.getCourses().size());

        try {
            StableMatchingResponseDTO response = stableMatchingService.solveStableMatching(request);
            this.lastResult = response; // Store result for querying
            
            if ("SUCCESS".equals(response.getStatus())) {
                log.info("Stable matching solved successfully in {} ms", 
                        response.getExecutionTimeMs());
                return ResponseEntity.ok(response);
            } else {
                log.warn("Stable matching failed: {}", response.getMessage());
                return ResponseEntity.badRequest().body(response);
            }
        } catch (Exception e) {
            log.error("Unexpected error during stable matching", e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(StableMatchingResponseDTO.builder()
                            .status("ERROR")
                            .message("Internal server error: " + e.getMessage())
                            .executionTimeMs(0L)
                            .build());
        }
    }

    /**
     * Get all assignments from the last solved matching
     *
     * @return list of all assignments
     */
    @GetMapping("/assignments")
    public ResponseEntity<List<StableMatchingResponseDTO.Assignment>> getAllAssignments() {
        log.info("GET /v1/matching/assignments - Retrieving all assignments");
        
        if (lastResult == null || lastResult.getAssignments() == null) {
            return ResponseEntity.ok(List.of());
        }
        return ResponseEntity.ok(lastResult.getAssignments());
    }

    /**
     * Get assignments for a specific student
     *
     * @param studentId the student ID
     * @return assignment for the student, if exists
     */
    @GetMapping("/assignments/student/{studentId}")
    public ResponseEntity<StableMatchingResponseDTO.Assignment> getStudentAssignment(
            @PathVariable String studentId) {
        log.info("GET /v1/matching/assignments/student/{} - Retrieving assignment", studentId);
        
        if (lastResult == null || lastResult.getAssignments() == null) {
            return ResponseEntity.notFound().build();
        }

        return lastResult.getAssignments().stream()
                .filter(a -> a.getStudentId().equals(studentId))
                .findFirst()
                .map(ResponseEntity::ok)
                .orElse(ResponseEntity.notFound().build());
    }

    /**
     * Get all assignments for a specific course
     *
     * @param courseId the course ID
     * @return list of assignments for the course
     */
    @GetMapping("/assignments/course/{courseId}")
    public ResponseEntity<List<StableMatchingResponseDTO.Assignment>> getCourseAssignments(
            @PathVariable String courseId) {
        log.info("GET /v1/matching/assignments/course/{} - Retrieving assignments", courseId);
        
        if (lastResult == null || lastResult.getAssignments() == null) {
            return ResponseEntity.ok(List.of());
        }

        List<StableMatchingResponseDTO.Assignment> courseAssignments = lastResult.getAssignments().stream()
                .filter(a -> a.getCourseId().equals(courseId))
                .collect(Collectors.toList());

        return ResponseEntity.ok(courseAssignments);
    }

    /**
     * Get unmatched students from the last matching
     *
     * @return list of student IDs that couldn't be matched
     */
    @GetMapping("/unmatched-students")
    public ResponseEntity<List<String>> getUnmatchedStudents() {
        log.info("GET /v1/matching/unmatched-students - Retrieving unmatched students");
        
        if (lastResult == null || lastResult.getUnmatchedStudents() == null) {
            return ResponseEntity.ok(List.of());
        }
        return ResponseEntity.ok(lastResult.getUnmatchedStudents());
    }

    /**
     * Get full courses from the last matching
     *
     * @return list of course IDs at capacity
     */
    @GetMapping("/full-courses")
    public ResponseEntity<List<String>> getFullCourses() {
        log.info("GET /v1/matching/full-courses - Retrieving full courses");
        
        if (lastResult == null || lastResult.getFullCourses() == null) {
            return ResponseEntity.ok(List.of());
        }
        return ResponseEntity.ok(lastResult.getFullCourses());
    }

    /**
     * Get matching result summary
     *
     * @return summary information
     */
    @GetMapping("/result-summary")
    public ResponseEntity<MatchingSummaryDTO> getResultSummary() {
        log.info("GET /v1/matching/result-summary - Retrieving result summary");
        
        if (lastResult == null) {
            return ResponseEntity.ok(MatchingSummaryDTO.builder()
                    .status("NO_RESULT")
                    .totalAssignments(0)
                    .unmatchedCount(0)
                    .fullCoursesCount(0)
                    .executionTimeMs(0L)
                    .build());
        }

        return ResponseEntity.ok(MatchingSummaryDTO.builder()
                .status(lastResult.getStatus())
                .message(lastResult.getMessage())
                .totalAssignments(lastResult.getAssignments() != null ? lastResult.getAssignments().size() : 0)
                .unmatchedCount(lastResult.getUnmatchedStudents() != null ? lastResult.getUnmatchedStudents().size() : 0)
                .fullCoursesCount(lastResult.getFullCourses() != null ? lastResult.getFullCourses().size() : 0)
                .executionTimeMs(lastResult.getExecutionTimeMs())
                .build());
    }

    /**
     * Health check endpoint
     *
     * @return status message
     */
    @GetMapping("/health")
    public ResponseEntity<String> health() {
        return ResponseEntity.ok("StableMatch service is running");
    }

    /**
     * API information endpoint
     *
     * @return API version and description
     */
    @GetMapping("/info")
    public ResponseEntity<ApiInfoDTO> info() {
        return ResponseEntity.ok(ApiInfoDTO.builder()
                .name("StableMatch API")
                .version("1.0.0")
                .description("Stable Matching Algorithm for Student Course Assignment")
                .endpoint("/v1/matching/solve")
                .method("POST")
                .build());
    }

    /**
     * Data Transfer Object for API information
     */
    @lombok.Data
    @lombok.NoArgsConstructor
    @lombok.AllArgsConstructor
    @lombok.Builder
    public static class ApiInfoDTO {
        private String name;
        private String version;
        private String description;
        private String endpoint;
        private String method;
    }

    /**
     * Data Transfer Object for matching result summary
     */
    @lombok.Data
    @lombok.NoArgsConstructor
    @lombok.AllArgsConstructor
    @lombok.Builder
    public static class MatchingSummaryDTO {
        private String status;
        private String message;
        private int totalAssignments;
        private int unmatchedCount;
        private int fullCoursesCount;
        private Long executionTimeMs;
    }
}
