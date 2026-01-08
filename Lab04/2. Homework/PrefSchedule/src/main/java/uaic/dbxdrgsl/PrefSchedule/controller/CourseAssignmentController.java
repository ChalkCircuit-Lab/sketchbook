package uaic.dbxdrgsl.PrefSchedule.controller;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.*;
import com.stablematch.dto.StableMatchingResponseDTO;
import uaic.dbxdrgsl.PrefSchedule.service.CourseAssignmentOrchestrationService;

import java.util.Map;

/**
 * REST Controller for managing course assignments.
 * Orchestrates the assignment workflow for optional courses.
 */
@Slf4j
@RestController
@RequestMapping("/api/assignments")
@RequiredArgsConstructor
@CrossOrigin(origins = "*", maxAge = 3600)
public class CourseAssignmentController {

    private final CourseAssignmentOrchestrationService orchestrationService;

    /**
     * Execute the complete assignment workflow.
     * Processes all optional courses in batches and invokes StableMatch service.
     *
     * @param batchSize number of courses per batch (default: 5)
     * @return map of batch results
     */
    @PostMapping("/execute-workflow")
    @PreAuthorize("hasAnyRole('ADMIN')")
    public ResponseEntity<AssignmentWorkflowResponse> executeAssignmentWorkflow(
            @RequestParam(defaultValue = "5") int batchSize) {

        log.info("POST /api/assignments/execute-workflow - Starting assignment workflow with batch size: {}", batchSize);

        if (batchSize <= 0) {
            return ResponseEntity.badRequest().body(AssignmentWorkflowResponse.builder()
                    .status("FAILED")
                    .message("Batch size must be greater than 0")
                    .totalBatches(0)
                    .successfulBatches(0)
                    .build());
        }

        try {
            long startTime = System.currentTimeMillis();
            Map<Integer, StableMatchingResponseDTO> results =
                    orchestrationService.executeAssignmentWorkflow(batchSize);

            long executionTime = System.currentTimeMillis() - startTime;

            int successfulBatches = (int) results.values().stream()
                    .filter(r -> "SUCCESS".equals(r.getStatus()))
                    .count();

            return ResponseEntity.ok(AssignmentWorkflowResponse.builder()
                    .status("COMPLETED")
                    .message("Assignment workflow executed successfully")
                    .totalBatches(results.size())
                    .successfulBatches(successfulBatches)
                    .executionTimeMs(executionTime)
                    .results(results)
                    .build());

        } catch (Exception e) {
            log.error("Error executing assignment workflow", e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(AssignmentWorkflowResponse.builder()
                            .status("FAILED")
                            .message("Error executing workflow: " + e.getMessage())
                            .totalBatches(0)
                            .successfulBatches(0)
                            .build());
        }
    }

    /**
     * Data Transfer Object for assignment workflow response
     */
    @lombok.Data
    @lombok.NoArgsConstructor
    @lombok.AllArgsConstructor
    @lombok.Builder
    public static class AssignmentWorkflowResponse {
        private String status;
        private String message;
        private int totalBatches;
        private int successfulBatches;
        private Long executionTimeMs;
        private Map<Integer, StableMatchingResponseDTO> results;
    }
}
