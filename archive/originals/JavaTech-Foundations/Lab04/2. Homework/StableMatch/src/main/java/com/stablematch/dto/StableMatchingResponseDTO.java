package com.stablematch.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.List;
import java.util.Map;

/**
 * Data Transfer Object representing the solution to a stable matching problem.
 * Contains the final assignments of students to courses.
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class StableMatchingResponseDTO {

    /**
     * Status of the matching algorithm execution
     */
    @JsonProperty("status")
    private String status;

    /**
     * Human-readable message about the result
     */
    @JsonProperty("message")
    private String message;

    /**
     * List of successful assignments
     */
    @JsonProperty("assignments")
    private List<Assignment> assignments;

    /**
     * List of students who could not be assigned to any course
     */
    @JsonProperty("unmatched_students")
    private List<String> unmatchedStudents;

    /**
     * List of courses with no capacity left
     */
    @JsonProperty("full_courses")
    private List<String> fullCourses;

    /**
     * Algorithm execution time in milliseconds
     */
    @JsonProperty("execution_time_ms")
    private Long executionTimeMs;

    /**
     * Represents a single assignment of a student to a course.
     */
    @Data
    @NoArgsConstructor
    @AllArgsConstructor
    @Builder
    public static class Assignment {
        /**
         * Unique identifier for the student
         */
        @JsonProperty("student_id")
        private String studentId;

        /**
         * Unique identifier for the assigned course
         */
        @JsonProperty("course_id")
        private String courseId;

        /**
         * Ranking preference of this course for the student
         * (1 = most preferred, 2 = second preferred, etc.)
         */
        @JsonProperty("student_preference_rank")
        private Integer studentPreferenceRank;

        /**
         * Ranking preference of this student for the course
         * (1 = most preferred, 2 = second preferred, etc.)
         */
        @JsonProperty("course_preference_rank")
        private Integer coursePreferenceRank;
    }
}
