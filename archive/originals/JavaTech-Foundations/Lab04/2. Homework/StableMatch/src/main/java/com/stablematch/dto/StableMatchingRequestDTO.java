package com.stablematch.dto;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import com.fasterxml.jackson.annotation.JsonProperty;
import java.util.List;
import java.util.Map;

/**
 * Data Transfer Object representing a stable matching problem.
 * Contains students with their course preferences and courses with their student preferences.
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class StableMatchingRequestDTO {

    /**
     * List of students with their preference rankings for courses.
     * Each student has a unique ID and an ordered list of preferred courses.
     */
    @JsonProperty("students")
    private List<StudentPreference> students;

    /**
     * List of courses with their preference rankings for students.
     * Each course has a unique ID and an ordered list of preferred students.
     */
    @JsonProperty("courses")
    private List<CoursePreference> courses;

    /**
     * Optional: Maximum number of students per course (default: 1).
     * Represents the capacity of each course.
     */
    @JsonProperty("capacity_per_course")
    private Integer capacityPerCourse;

    /**
     * Represents a student's preference list for courses.
     */
    @Data
    @NoArgsConstructor
    @AllArgsConstructor
    @Builder
    public static class StudentPreference {
        /**
         * Unique identifier for the student
         */
        @JsonProperty("student_id")
        private String studentId;

        /**
         * Ordered list of course IDs ranked from most to least preferred
         */
        @JsonProperty("preferences")
        private List<String> preferences;
    }

    /**
     * Represents a course's preference list for students.
     */
    @Data
    @NoArgsConstructor
    @AllArgsConstructor
    @Builder
    public static class CoursePreference {
        /**
         * Unique identifier for the course
         */
        @JsonProperty("course_id")
        private String courseId;

        /**
         * Ordered list of student IDs ranked from most to least preferred
         */
        @JsonProperty("preferences")
        private List<String> preferences;
    }
}
