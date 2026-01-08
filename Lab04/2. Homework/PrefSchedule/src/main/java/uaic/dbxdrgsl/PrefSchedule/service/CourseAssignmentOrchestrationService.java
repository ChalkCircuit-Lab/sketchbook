package uaic.dbxdrgsl.PrefSchedule.service;

import com.stablematch.dto.StableMatchingRequestDTO;
import com.stablematch.dto.StableMatchingResponseDTO;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import uaic.dbxdrgsl.PrefSchedule.model.*;
import uaic.dbxdrgsl.PrefSchedule.repository.*;

import java.*;
import java.util.*;
import java.util.stream.Collectors;

/**
 * Service for orchestrating the course assignment process.
 * Handles batching of optional courses, calculating student preferences based on grades,
 * and invoking the StableMatch service.
 */
@Slf4j
@Service
@RequiredArgsConstructor
@Transactional
public class CourseAssignmentOrchestrationService {

    private final StableMatchClient stableMatchClient;
    private final CourseRepository courseRepository;
    private final StudentRepository studentRepository;
    private final GradeRepository gradeRepository;
    private final InstructorCoursePreferenceRepository instructorPreferenceRepository;
    private final EnrollmentRepository enrollmentRepository;

    /**
     * Execute the complete assignment workflow for optional courses
     * Batches optional courses and invokes StableMatch service for each batch
     *
     * @param batchSize number of optional courses per batch
     * @return map of batch ID to matching results
     */
    public Map<Integer, StableMatchingResponseDTO> executeAssignmentWorkflow(int batchSize) {
        log.info("Starting course assignment workflow with batch size: {}", batchSize);

        long startTime = System.currentTimeMillis();
        Map<Integer, StableMatchingResponseDTO> results = new HashMap<>();

        try {
            // Get all optional courses
            List<Course> optionalCourses = courseRepository.findAll().stream()
                    .filter(c -> !c.isCompulsory())
                    .collect(Collectors.toList());

            if (optionalCourses.isEmpty()) {
                log.warn("No optional courses found");
                return results;
            }

            log.info("Found {} optional courses", optionalCourses.size());

            // Get all students
            List<Student> students = studentRepository.findAll();
            if (students.isEmpty()) {
                log.warn("No students found");
                return results;
            }

            log.info("Found {} students", students.size());

            // Batch optional courses
            List<List<Course>> batches = batchOptionalCourses(optionalCourses, batchSize);
            log.info("Created {} batches of optional courses", batches.size());

            // Process each batch
            for (int i = 0; i < batches.size(); i++) {
                List<Course> batch = batches.get(i);
                log.info("Processing batch {} with {} courses", i + 1, batch.size());

                StableMatchingResponseDTO result = processBatch(batch, students);
                results.put(i, result);

                // Save assignments to database
                if ("SUCCESS".equals(result.getStatus())) {
                    saveAssignmentsToDatabase(result, batch);
                    log.info("Batch {} completed with {} successful assignments",
                            i + 1, result.getAssignments().size());
                } else {
                    log.warn("Batch {} failed: {}", i + 1, result.getMessage());
                }
            }

            long totalTime = System.currentTimeMillis() - startTime;
            log.info("Course assignment workflow completed in {} ms with {} batches",
                    totalTime, results.size());

        } catch (Exception e) {
            log.error("Error executing assignment workflow", e);
        }

        return results;
    }

    /**
     * Process a single batch of optional courses
     * Builds student preferences and course preferences for the batch
     *
     * @param optionalCourses courses in this batch
     * @param students all students
     * @return the matching result from StableMatch service
     */
    private StableMatchingResponseDTO processBatch(
            List<Course> optionalCourses,
            List<Student> students) {

        log.info("Processing batch with {} optional courses and {} students",
                optionalCourses.size(), students.size());

        // Build course preferences based on student grades in compulsory courses
        Map<String, StudentScoreInfo> studentScores = calculateStudentScoresForBatch(
                optionalCourses, students);

        // Build student preferences for this batch
        List<StableMatchingRequestDTO.StudentPreference> studentPreferences =
                buildStudentPreferencesForBatch(optionalCourses, students);

        // Build course preferences based on calculated student scores
        List<StableMatchingRequestDTO.CoursePreference> coursePreferences =
                buildCoursePreferencesForBatch(optionalCourses, studentScores);

        // Create request for StableMatch service
        StableMatchingRequestDTO request = StableMatchingRequestDTO.builder()
                .students(studentPreferences)
                .courses(coursePreferences)
                .capacityPerCourse(1)
                .build();

        // Invoke StableMatch service
        log.info("Invoking StableMatch service for batch");
        return stableMatchClient.solveMatching(request);
    }

    /**
     * Calculate student scores for each optional course based on
     * instructor preferences and student grades in compulsory courses
     *
     * @param optionalCourses courses in the batch
     * @param students all students
     * @return map of student ID to their scores for each course
     */
    private Map<String, StudentScoreInfo> calculateStudentScoresForBatch(
            List<Course> optionalCourses,
            List<Student> students) {

        Map<String, StudentScoreInfo> scores = new HashMap<>();

        for (Student student : students) {
            StudentScoreInfo scoreInfo = new StudentScoreInfo();
            String studentCode = student.getStudentNumber();

            for (Course course : optionalCourses) {
                // Get instructor preferences for this optional course
                List<InstructorCoursePreference> preferences =
                        instructorPreferenceRepository.findByOptionalCourseId(course.getId());

                if (preferences.isEmpty()) {
                    // No preferences specified, give neutral score
                    scoreInfo.scores.put(course.getCode(), 50.0);
                    continue;
                }

                // Calculate weighted average of grades
                double weightedSum = 0.0;
                double totalWeight = 0.0;

                for (InstructorCoursePreference pref : preferences) {
                    double weight = pref.getPercentage();
                    // Get student's grade in this compulsory course
                    List<Grade> grades = gradeRepository.findAll().stream()
                            .filter(g -> g.getStudentCode().equals(studentCode)
                                    && g.getCourseCode().equals(pref.getCompulsoryCourseCode()))
                            .collect(Collectors.toList());

                    if (!grades.isEmpty()) {
                        // Use most recent grade
                        double grade = grades.stream()
                                .max(Comparator.comparing(Grade::getReceivedAt))
                                .map(Grade::getGrade)
                                .orElse(0.0);

                        weightedSum += grade * weight;
                        totalWeight += weight;
                    }
                }

                double finalScore = totalWeight > 0 ? weightedSum / totalWeight : 50.0;
                scoreInfo.scores.put(course.getCode(), finalScore);
                log.debug("Student {} score for course {}: {}", studentCode, course.getCode(), finalScore);
            }

            scores.put(studentCode, scoreInfo);
        }

        return scores;
    }

    /**
     * Build student preferences for the batch.
     * For now, preferences are ordered by course ID (can be enhanced with student preferences)
     *
     * @param optionalCourses courses in the batch
     * @param students all students
     * @return list of student preferences
     */
    private List<StableMatchingRequestDTO.StudentPreference> buildStudentPreferencesForBatch(
            List<Course> optionalCourses,
            List<Student> students) {

        List<String> courseIds = optionalCourses.stream()
                .map(Course::getCode)
                .sorted()
                .collect(Collectors.toList());

        return students.stream()
                .map(student -> StableMatchingRequestDTO.StudentPreference.builder()
                        .studentId(student.getStudentNumber())
                        .preferences(new ArrayList<>(courseIds))
                        .build())
                .collect(Collectors.toList());
    }

    /**
     * Build course preferences for the batch based on student scores.
     * Courses prefer students with higher weighted scores.
     *
     * @param optionalCourses courses in the batch
     * @param studentScores calculated scores for each student
     * @return list of course preferences
     */
    private List<StableMatchingRequestDTO.CoursePreference> buildCoursePreferencesForBatch(
            List<Course> optionalCourses,
            Map<String, StudentScoreInfo> studentScores) {

        List<StableMatchingRequestDTO.CoursePreference> preferences = new ArrayList<>();

        for (Course course : optionalCourses) {
            // Sort students by their score for this course (descending)
            List<String> rankedStudents = studentScores.entrySet().stream()
                    .sorted((e1, e2) -> Double.compare(
                            e2.getValue().scores.getOrDefault(course.getCode(), 0.0),
                            e1.getValue().scores.getOrDefault(course.getCode(), 0.0)))
                    .map(Map.Entry::getKey)
                    .collect(Collectors.toList());

            preferences.add(StableMatchingRequestDTO.CoursePreference.builder()
                    .courseId(course.getCode())
                    .preferences(rankedStudents)
                    .build());
        }

        return preferences;
    }

    /**
     * Save the matching results to the database by creating enrollments
     *
     * @param result the matching result from StableMatch
     * @param courses the courses in this batch
     */
    private void saveAssignmentsToDatabase(
            StableMatchingResponseDTO result,
            List<Course> courses) {

        log.info("Saving {} assignments to database", result.getAssignments().size());

        Map<String, Course> courseMap = courses.stream()
                .collect(Collectors.toMap(Course::getCode, c -> c));

        Map<Long, Student> studentMap = studentRepository.findAll().stream()
                .collect(Collectors.toMap(Student::getId, s -> s));

        for (StableMatchingResponseDTO.Assignment assignment : result.getAssignments()) {
            try {
                Course course = courseMap.get(assignment.getCourseId());
                if (course == null) {
                    log.warn("Course not found: {}", assignment.getCourseId());
                    continue;
                }

                Student student = studentRepository.findAll().stream()
                        .filter(s -> s.getStudentNumber().equals(assignment.getStudentId()))
                        .findFirst()
                        .orElse(null);

                if (student == null) {
                    log.warn("Student not found: {}", assignment.getStudentId());
                    continue;
                }

                // Check if enrollment already exists
                if (enrollmentRepository.findAll().stream()
                        .noneMatch(e -> e.getStudent().getId().equals(student.getId())
                                && e.getCourse().getId().equals(course.getId()))) {

                    Enrollment enrollment = new Enrollment();
                    enrollment.setStudent(student);
                    enrollment.setCourse(course);
                    enrollmentRepository.save(enrollment);
                    log.debug("Created enrollment for student {} in course {}",
                            student.getStudentNumber(), course.getCode());
                }
            } catch (Exception e) {
                log.error("Error saving assignment: {}", e.getMessage());
            }
        }
    }

    /**
     * Batch optional courses into groups of specified size
     *
     * @param courses the courses to batch
     * @param batchSize the size of each batch
     * @return list of batches
     */
    private List<List<Course>> batchOptionalCourses(List<Course> courses, int batchSize) {
        List<List<Course>> batches = new ArrayList<>();
        for (int i = 0; i < courses.size(); i += batchSize) {
            batches.add(new ArrayList<>(courses.subList(i,
                    Math.min(i + batchSize, courses.size()))));
        }
        return batches;
    }

    /**
     * Helper class to store student scores for different courses
     */
    private static class StudentScoreInfo {
        Map<String, Double> scores = new HashMap<>();
    }
}
