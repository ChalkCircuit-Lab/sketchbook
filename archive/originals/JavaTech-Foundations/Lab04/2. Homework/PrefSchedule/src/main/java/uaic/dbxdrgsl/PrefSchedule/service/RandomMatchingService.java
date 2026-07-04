package uaic.dbxdrgsl.PrefSchedule.service;

import com.stablematch.dto.StableMatchingRequestDTO;
import com.stablematch.dto.StableMatchingResponseDTO;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Service implementing a random matching algorithm as a fallback mechanism.
 * Disregards preferences and simply assigns students to courses randomly.
 * Useful when the stable matching algorithm is unavailable.
 */
@Slf4j
@Service
public class RandomMatchingService {

    /**
     * Generate a random matching between students and optional courses.
     * Each student is assigned to a random course (if capacity allows).
     *
     * @param request the matching problem (preferences are ignored)
     * @return a random matching result
     */
    public StableMatchingResponseDTO generateRandomMatching(StableMatchingRequestDTO request) {
        long startTime = System.currentTimeMillis();
        log.info("Starting random matching algorithm for {} students and {} courses",
                request.getStudents().size(), request.getCourses().size());

        try {
            int capacity = request.getCapacityPerCourse() != null ?
                    request.getCapacityPerCourse() : 1;

            // Initialize course assignments
            Map<String, List<String>> courseAssignments = new HashMap<>();
            for (StableMatchingRequestDTO.CoursePreference course : request.getCourses()) {
                courseAssignments.put(course.getCourseId(), new ArrayList<>());
            }

            // Get all courses
            List<String> courseIds = request.getCourses().stream()
                    .map(StableMatchingRequestDTO.CoursePreference::getCourseId)
                    .collect(Collectors.toList());

            // Randomly assign each student to a course
            Random random = new Random();
            Set<String> assignedStudents = new HashSet<>();
            int attempts = 0;
            int maxAttempts = request.getStudents().size() * courseIds.size();

            for (StableMatchingRequestDTO.StudentPreference student : request.getStudents()) {
                String studentId = student.getStudentId();
                boolean assigned = false;
                attempts = 0;

                // Try to find a course with available capacity
                while (!assigned && attempts < maxAttempts) {
                    int randomIndex = random.nextInt(courseIds.size());
                    String courseId = courseIds.get(randomIndex);
                    List<String> courseStudents = courseAssignments.get(courseId);

                    if (courseStudents.size() < capacity) {
                        courseStudents.add(studentId);
                        assignedStudents.add(studentId);
                        assigned = true;
                        log.debug("Randomly assigned student {} to course {}", studentId, courseId);
                    }
                    attempts++;
                }

                if (!assigned) {
                    log.debug("Could not assign student {} to any course (no available capacity)", studentId);
                }
            }

            // Build response
            List<StableMatchingResponseDTO.Assignment> assignments = buildAssignments(
                    courseAssignments, request);

            Set<String> allStudents = request.getStudents().stream()
                    .map(StableMatchingRequestDTO.StudentPreference::getStudentId)
                    .collect(Collectors.toSet());

            List<String> unmatchedStudents = allStudents.stream()
                    .filter(s -> !assignedStudents.contains(s))
                    .collect(Collectors.toList());

            long executionTime = System.currentTimeMillis() - startTime;

            log.info("Random matching completed: {} assignments, {} unmatched students in {} ms",
                    assignments.size(), unmatchedStudents.size(), executionTime);

            return StableMatchingResponseDTO.builder()
                    .status("SUCCESS")
                    .message("Random matching completed (fallback algorithm)")
                    .assignments(assignments)
                    .unmatchedStudents(unmatchedStudents)
                    .fullCourses(courseAssignments.entrySet().stream()
                            .filter(e -> e.getValue().size() >= capacity)
                            .map(Map.Entry::getKey)
                            .collect(Collectors.toList()))
                    .executionTimeMs(executionTime)
                    .build();

        } catch (Exception e) {
            log.error("Error in random matching algorithm", e);
            long executionTime = System.currentTimeMillis() - startTime;
            return StableMatchingResponseDTO.builder()
                    .status("ERROR")
                    .message("Random matching failed: " + e.getMessage())
                    .assignments(new ArrayList<>())
                    .unmatchedStudents(new ArrayList<>())
                    .fullCourses(new ArrayList<>())
                    .executionTimeMs(executionTime)
                    .build();
        }
    }

    private List<StableMatchingResponseDTO.Assignment> buildAssignments(
            Map<String, List<String>> courseAssignments,
            StableMatchingRequestDTO request) {

        List<StableMatchingResponseDTO.Assignment> assignments = new ArrayList<>();

        for (Map.Entry<String, List<String>> entry : courseAssignments.entrySet()) {
            String course = entry.getKey();
            List<String> students = entry.getValue();

            for (String student : students) {
                assignments.add(StableMatchingResponseDTO.Assignment.builder()
                        .studentId(student)
                        .courseId(course)
                        .studentPreferenceRank(-1) // Not calculated for random matching
                        .coursePreferenceRank(-1)
                        .build());
            }
        }

        return assignments.stream()
                .sorted(Comparator.comparing(StableMatchingResponseDTO.Assignment::getStudentId))
                .collect(Collectors.toList());
    }
}
