package com.stablematch.algorithm;

import com.stablematch.dto.StableMatchingRequestDTO;
import com.stablematch.dto.StableMatchingResponseDTO;
import io.micrometer.core.instrument.Counter;
import io.micrometer.core.instrument.MeterRegistry;
import io.micrometer.core.instrument.Timer;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Service implementing the Gale-Shapley algorithm for stable matching
 * in the context of student-course assignment.
 */
@Slf4j
@Service
public class StableMatchingService {

    private final MeterRegistry meterRegistry;
    private final Counter stableMatchInvocations;
    private final Timer stableMatchTimer;

    public StableMatchingService(MeterRegistry meterRegistry) {
        this.meterRegistry = meterRegistry;
        this.stableMatchInvocations = Counter.builder("stablematch.solve.invocations")
                .description("Number of times the stable matching algorithm is invoked")
                .register(meterRegistry);
        this.stableMatchTimer = Timer.builder("stablematch.solve.duration")
                .description("Duration of stable matching algorithm execution")
                .publishPercentileHistogram(true)
                .register(meterRegistry);
    }

    /**
     * Solves the stable matching problem using a variant of the Gale-Shapley algorithm
     * optimized for many-to-one matching (courses can accept multiple students).
     *
     * @param request the stable matching problem definition
     * @return the solution with student-to-course assignments
     */
    public StableMatchingResponseDTO solveStableMatching(StableMatchingRequestDTO request) {
        long startTime = System.currentTimeMillis();
        Timer.Sample timerSample = Timer.start(meterRegistry);
        stableMatchInvocations.increment();

        log.info("Starting stable matching: students={}, courses={}, capacityPerCourse={}",
                request.getStudents() != null ? request.getStudents().size() : 0,
                request.getCourses() != null ? request.getCourses().size() : 0,
                request.getCapacityPerCourse());

        try {
            // Validate input
            validateRequest(request);

            // Determine course capacity
            int capacity = request.getCapacityPerCourse() != null ? 
                    request.getCapacityPerCourse() : 1;

            // Initialize data structures
            Map<String, List<String>> studentPreferences = buildStudentPreferenceMap(request);
            Map<String, List<String>> coursePreferences = buildCoursePreferenceMap(request);
            Map<String, Integer> courseRankings = buildCourseRankings(coursePreferences);

            // Run the matching algorithm
            Map<String, List<String>> courseAssignments = new HashMap<>();
            for (StableMatchingRequestDTO.CoursePreference course : request.getCourses()) {
                courseAssignments.put(course.getCourseId(), new ArrayList<>());
            }

            Queue<String> freeStudents = new LinkedList<>(
                    request.getStudents().stream()
                            .map(StableMatchingRequestDTO.StudentPreference::getStudentId)
                            .collect(Collectors.toList())
            );

            while (!freeStudents.isEmpty()) {
                String student = freeStudents.poll();
                List<String> preferences = studentPreferences.get(student);

                for (String course : preferences) {
                    List<String> courseStudents = courseAssignments.get(course);

                    if (courseStudents.size() < capacity) {
                        // Course has space, assign student
                        courseStudents.add(student);
                        break;
                    } else {
                        // Course is full, try to replace weakest match
                        String worstStudentInCourse = findWorstMatchInCourse(
                                courseStudents, course, courseRankings
                        );

                        String worst = courseStudents.get(courseStudents.size() - 1);
                        int worstRank = courseRankings.getOrDefault(
                                course + "_" + worst, 
                                Integer.MAX_VALUE
                        );
                        int newRank = courseRankings.getOrDefault(
                                course + "_" + student, 
                                Integer.MAX_VALUE
                        );

                        if (newRank < worstRank) {
                            courseStudents.remove(worst);
                            courseStudents.add(student);
                            freeStudents.offer(worst);
                            break;
                        }
                    }
                }
            }

            // Build response
            List<StableMatchingResponseDTO.Assignment> assignments = buildAssignments(
                    courseAssignments, studentPreferences, coursePreferences
            );

            Set<String> allStudents = request.getStudents().stream()
                    .map(StableMatchingRequestDTO.StudentPreference::getStudentId)
                    .collect(Collectors.toSet());

            Set<String> assignedStudents = assignments.stream()
                    .map(StableMatchingResponseDTO.Assignment::getStudentId)
                    .collect(Collectors.toSet());

            List<String> unmatchedStudents = allStudents.stream()
                    .filter(s -> !assignedStudents.contains(s))
                    .collect(Collectors.toList());

            long executionTime = System.currentTimeMillis() - startTime;

                StableMatchingResponseDTO result = StableMatchingResponseDTO.builder()
                    .status("SUCCESS")
                    .message("Stable matching completed successfully")
                    .assignments(assignments)
                    .unmatchedStudents(unmatchedStudents)
                    .fullCourses(courseAssignments.entrySet().stream()
                            .filter(e -> e.getValue().size() >= capacity)
                            .map(Map.Entry::getKey)
                            .collect(Collectors.toList()))
                    .executionTimeMs(executionTime)
                    .build();
                timerSample.stop(stableMatchTimer);
                log.info("Stable matching success: assignments={}, unmatched={}, fullCourses={}, durationMs={}",
                    result.getAssignments() != null ? result.getAssignments().size() : 0,
                    result.getUnmatchedStudents() != null ? result.getUnmatchedStudents().size() : 0,
                    result.getFullCourses() != null ? result.getFullCourses().size() : 0,
                    result.getExecutionTimeMs());
                return result;

        } catch (IllegalArgumentException e) {
            long executionTime = System.currentTimeMillis() - startTime;
                timerSample.stop(stableMatchTimer);
                log.warn("Stable matching validation failed: {}", e.getMessage());
                return StableMatchingResponseDTO.builder()
                    .status("ERROR")
                    .message("Invalid input: " + e.getMessage())
                    .assignments(new ArrayList<>())
                    .unmatchedStudents(new ArrayList<>())
                    .fullCourses(new ArrayList<>())
                    .executionTimeMs(executionTime)
                    .build();
        } catch (Exception e) {
            long executionTime = System.currentTimeMillis() - startTime;
            timerSample.stop(stableMatchTimer);
            log.error("Stable matching unexpected error", e);
            return StableMatchingResponseDTO.builder()
                    .status("ERROR")
                    .message("Internal error: " + e.getMessage())
                    .assignments(new ArrayList<>())
                    .unmatchedStudents(new ArrayList<>())
                    .fullCourses(new ArrayList<>())
                    .executionTimeMs(executionTime)
                    .build();
        }
    }

    private void validateRequest(StableMatchingRequestDTO request) {
        if (request.getStudents() == null || request.getStudents().isEmpty()) {
            throw new IllegalArgumentException("Students list cannot be empty");
        }
        if (request.getCourses() == null || request.getCourses().isEmpty()) {
            throw new IllegalArgumentException("Courses list cannot be empty");
        }

        Set<String> studentIds = new HashSet<>();
        for (StableMatchingRequestDTO.StudentPreference sp : request.getStudents()) {
            if (sp.getStudentId() == null || sp.getStudentId().isEmpty()) {
                throw new IllegalArgumentException("Student ID cannot be null or empty");
            }
            if (!studentIds.add(sp.getStudentId())) {
                throw new IllegalArgumentException("Duplicate student ID: " + sp.getStudentId());
            }
            if (sp.getPreferences() == null || sp.getPreferences().isEmpty()) {
                throw new IllegalArgumentException("Student " + sp.getStudentId() + 
                        " must have at least one preference");
            }
        }

        Set<String> courseIds = new HashSet<>();
        for (StableMatchingRequestDTO.CoursePreference cp : request.getCourses()) {
            if (cp.getCourseId() == null || cp.getCourseId().isEmpty()) {
                throw new IllegalArgumentException("Course ID cannot be null or empty");
            }
            if (!courseIds.add(cp.getCourseId())) {
                throw new IllegalArgumentException("Duplicate course ID: " + cp.getCourseId());
            }
            if (cp.getPreferences() == null || cp.getPreferences().isEmpty()) {
                throw new IllegalArgumentException("Course " + cp.getCourseId() + 
                        " must have at least one preference");
            }
        }
    }

    private Map<String, List<String>> buildStudentPreferenceMap(StableMatchingRequestDTO request) {
        Map<String, List<String>> preferences = new HashMap<>();
        for (StableMatchingRequestDTO.StudentPreference sp : request.getStudents()) {
            preferences.put(sp.getStudentId(), new ArrayList<>(sp.getPreferences()));
        }
        return preferences;
    }

    private Map<String, List<String>> buildCoursePreferenceMap(StableMatchingRequestDTO request) {
        Map<String, List<String>> preferences = new HashMap<>();
        for (StableMatchingRequestDTO.CoursePreference cp : request.getCourses()) {
            preferences.put(cp.getCourseId(), new ArrayList<>(cp.getPreferences()));
        }
        return preferences;
    }

    private Map<String, Integer> buildCourseRankings(Map<String, List<String>> coursePreferences) {
        Map<String, Integer> rankings = new HashMap<>();
        for (Map.Entry<String, List<String>> entry : coursePreferences.entrySet()) {
            String course = entry.getKey();
            List<String> prefs = entry.getValue();
            for (int i = 0; i < prefs.size(); i++) {
                rankings.put(course + "_" + prefs.get(i), i + 1);
            }
        }
        return rankings;
    }

    private String findWorstMatchInCourse(List<String> students, String course, 
                                         Map<String, Integer> courseRankings) {
        return students.stream()
                .max(Comparator.comparingInt(s -> 
                        courseRankings.getOrDefault(course + "_" + s, Integer.MAX_VALUE)))
                .orElse(students.get(students.size() - 1));
    }

    private List<StableMatchingResponseDTO.Assignment> buildAssignments(
            Map<String, List<String>> courseAssignments,
            Map<String, List<String>> studentPreferences,
            Map<String, List<String>> coursePreferences) {

        List<StableMatchingResponseDTO.Assignment> assignments = new ArrayList<>();

        for (Map.Entry<String, List<String>> entry : courseAssignments.entrySet()) {
            String course = entry.getKey();
            List<String> students = entry.getValue();
            List<String> coursePrefs = coursePreferences.get(course);

            for (String student : students) {
                List<String> studentPrefs = studentPreferences.get(student);
                
                int studentRank = studentPrefs.indexOf(course) + 1;
                int courseRank = coursePrefs.indexOf(student) + 1;

                assignments.add(StableMatchingResponseDTO.Assignment.builder()
                        .studentId(student)
                        .courseId(course)
                        .studentPreferenceRank(studentRank)
                        .coursePreferenceRank(courseRank)
                        .build());
            }
        }

        return assignments.stream()
                .sorted(Comparator.comparing(StableMatchingResponseDTO.Assignment::getStudentId))
                .collect(Collectors.toList());
    }
}
