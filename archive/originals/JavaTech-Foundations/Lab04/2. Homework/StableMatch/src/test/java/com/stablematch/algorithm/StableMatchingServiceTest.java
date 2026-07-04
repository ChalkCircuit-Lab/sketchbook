package com.stablematch.algorithm;

import com.stablematch.dto.StableMatchingRequestDTO;
import com.stablematch.dto.StableMatchingResponseDTO;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.Arrays;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

class StableMatchingServiceTest {

    private StableMatchingService stableMatchingService;

    @BeforeEach
    void setUp() {
        stableMatchingService = new StableMatchingService();
    }

    @Test
    void testSimpleMatching() {
        // Create a simple matching problem
        StableMatchingRequestDTO request = StableMatchingRequestDTO.builder()
                .students(Arrays.asList(
                        StableMatchingRequestDTO.StudentPreference.builder()
                                .studentId("S1")
                                .preferences(Arrays.asList("C1", "C2"))
                                .build(),
                        StableMatchingRequestDTO.StudentPreference.builder()
                                .studentId("S2")
                                .preferences(Arrays.asList("C2", "C1"))
                                .build()
                ))
                .courses(Arrays.asList(
                        StableMatchingRequestDTO.CoursePreference.builder()
                                .courseId("C1")
                                .preferences(Arrays.asList("S1", "S2"))
                                .build(),
                        StableMatchingRequestDTO.CoursePreference.builder()
                                .courseId("C2")
                                .preferences(Arrays.asList("S2", "S1"))
                                .build()
                ))
                .capacityPerCourse(1)
                .build();

        StableMatchingResponseDTO response = stableMatchingService.solveStableMatching(request);

        assertEquals("SUCCESS", response.getStatus());
        assertEquals(2, response.getAssignments().size());
        assertTrue(response.getUnmatchedStudents().isEmpty());
    }

    @Test
    void testMultiCapacityMatching() {
        // Create a matching problem with multiple capacities per course
        StableMatchingRequestDTO request = StableMatchingRequestDTO.builder()
                .students(Arrays.asList(
                        StableMatchingRequestDTO.StudentPreference.builder()
                                .studentId("S1")
                                .preferences(Arrays.asList("C1"))
                                .build(),
                        StableMatchingRequestDTO.StudentPreference.builder()
                                .studentId("S2")
                                .preferences(Arrays.asList("C1"))
                                .build(),
                        StableMatchingRequestDTO.StudentPreference.builder()
                                .studentId("S3")
                                .preferences(Arrays.asList("C1"))
                                .build()
                ))
                .courses(Arrays.asList(
                        StableMatchingRequestDTO.CoursePreference.builder()
                                .courseId("C1")
                                .preferences(Arrays.asList("S1", "S2", "S3"))
                                .build()
                ))
                .capacityPerCourse(2)
                .build();

        StableMatchingResponseDTO response = stableMatchingService.solveStableMatching(request);

        assertEquals("SUCCESS", response.getStatus());
        assertEquals(2, response.getAssignments().size());
        assertEquals(1, response.getUnmatchedStudents().size());
    }

    @Test
    void testInvalidRequestEmptyStudents() {
        StableMatchingRequestDTO request = StableMatchingRequestDTO.builder()
                .students(Arrays.asList())
                .courses(Arrays.asList())
                .build();

        StableMatchingResponseDTO response = stableMatchingService.solveStableMatching(request);

        assertEquals("ERROR", response.getStatus());
        assertTrue(response.getMessage().contains("Students list cannot be empty"));
    }

    @Test
    void testExecutionTimeTracking() {
        StableMatchingRequestDTO request = StableMatchingRequestDTO.builder()
                .students(Arrays.asList(
                        StableMatchingRequestDTO.StudentPreference.builder()
                                .studentId("S1")
                                .preferences(Arrays.asList("C1"))
                                .build()
                ))
                .courses(Arrays.asList(
                        StableMatchingRequestDTO.CoursePreference.builder()
                                .courseId("C1")
                                .preferences(Arrays.asList("S1"))
                                .build()
                ))
                .capacityPerCourse(1)
                .build();

        StableMatchingResponseDTO response = stableMatchingService.solveStableMatching(request);

        assertNotNull(response.getExecutionTimeMs());
        assertTrue(response.getExecutionTimeMs() >= 0);
    }
}
