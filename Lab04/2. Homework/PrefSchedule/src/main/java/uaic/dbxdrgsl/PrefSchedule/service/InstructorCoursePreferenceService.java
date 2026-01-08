package uaic.dbxdrgsl.PrefSchedule.service;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import uaic.dbxdrgsl.PrefSchedule.dto.InstructorCoursePreferenceDTO;
import uaic.dbxdrgsl.PrefSchedule.dto.InstructorCoursePreferenceBatchDTO;
import uaic.dbxdrgsl.PrefSchedule.model.Course;
import uaic.dbxdrgsl.PrefSchedule.model.InstructorCoursePreference;
import uaic.dbxdrgsl.PrefSchedule.repository.CourseRepository;
import uaic.dbxdrgsl.PrefSchedule.repository.InstructorCoursePreferenceRepository;

import java.util.List;
import java.util.stream.Collectors;

@Slf4j
@Service
@RequiredArgsConstructor
@Transactional
public class InstructorCoursePreferenceService {

    private final InstructorCoursePreferenceRepository preferenceRepository;
    private final CourseRepository courseRepository;

    /**
     * Create a new instructor course preference
     */
    public InstructorCoursePreferenceDTO createPreference(InstructorCoursePreferenceDTO dto) {
        log.info("Creating instructor preference for optional course: {}, compulsory course: {}",
                dto.getOptionalCourseId(), dto.getCompulsoryCourseCode());

        Course optionalCourse = courseRepository.findById(dto.getOptionalCourseId())
                .orElseThrow(() -> new IllegalArgumentException("Course not found: " + dto.getOptionalCourseId()));

        if (optionalCourse.isCompulsory()) {
            throw new IllegalArgumentException("Can only create preferences for optional courses");
        }

        // Check if preference already exists
        if (preferenceRepository.existsByOptionalCourseIdAndCompulsoryCourseCode(
                dto.getOptionalCourseId(), dto.getCompulsoryCourseCode())) {
            throw new IllegalArgumentException("Preference already exists for this course pair");
        }

        InstructorCoursePreference preference = InstructorCoursePreference.builder()
                .optionalCourse(optionalCourse)
                .compulsoryCourseCode(dto.getCompulsoryCourseCode())
                .percentage(dto.getPercentage())
                .build();

        preference.validate();
        InstructorCoursePreference saved = preferenceRepository.save(preference);

        return mapToDTO(saved);
    }

    /**
     * Create multiple preferences for an optional course in one batch
     */
    public List<InstructorCoursePreferenceDTO> createBatchPreferences(InstructorCoursePreferenceBatchDTO batchDTO) {
        log.info("Creating batch of {} preferences for optional course: {}",
                batchDTO.getPreferences().size(), batchDTO.getOptionalCourseId());

        Course optionalCourse = courseRepository.findById(batchDTO.getOptionalCourseId())
                .orElseThrow(() -> new IllegalArgumentException("Course not found: " + batchDTO.getOptionalCourseId()));

        if (optionalCourse.isCompulsory()) {
            throw new IllegalArgumentException("Can only create preferences for optional courses");
        }

        // Delete existing preferences for this course
        List<InstructorCoursePreference> existingPrefs = preferenceRepository
                .findByOptionalCourseId(batchDTO.getOptionalCourseId());
        preferenceRepository.deleteAll(existingPrefs);

        // Validate total percentage adds up to 100
        double totalPercentage = batchDTO.getPreferences().stream()
                .mapToDouble(InstructorCoursePreferenceBatchDTO.PreferenceItem::getPercentage)
                .sum();

        if (totalPercentage > 100.0 + 0.01) { // Allow for small floating point errors
            throw new IllegalArgumentException("Total percentage cannot exceed 100%, got: " + totalPercentage);
        }

        // Create new preferences
        List<InstructorCoursePreference> preferences = batchDTO.getPreferences().stream()
                .map(item -> {
                    InstructorCoursePreference preference = InstructorCoursePreference.builder()
                            .optionalCourse(optionalCourse)
                            .compulsoryCourseCode(item.getCompulsoryCourseCode())
                            .percentage(item.getPercentage())
                            .build();
                    preference.validate();
                    return preference;
                })
                .collect(Collectors.toList());

        List<InstructorCoursePreference> saved = preferenceRepository.saveAll(preferences);
        return saved.stream().map(this::mapToDTO).collect(Collectors.toList());
    }

    /**
     * Get all preferences for a specific optional course
     */
    public List<InstructorCoursePreferenceDTO> getPreferencesForCourse(Long courseId) {
        log.info("Retrieving preferences for course: {}", courseId);
        return preferenceRepository.findByOptionalCourseId(courseId).stream()
                .map(this::mapToDTO)
                .collect(Collectors.toList());
    }

    /**
     * Get all preferences for a specific optional course by code
     */
    public List<InstructorCoursePreferenceDTO> getPreferencesForCourseCode(String courseCode) {
        log.info("Retrieving preferences for course code: {}", courseCode);
        return preferenceRepository.findByOptionalCourseCode(courseCode).stream()
                .map(this::mapToDTO)
                .collect(Collectors.toList());
    }

    /**
     * Update an existing preference
     */
    public InstructorCoursePreferenceDTO updatePreference(Long preferenceId, InstructorCoursePreferenceDTO dto) {
        log.info("Updating preference: {}", preferenceId);
        InstructorCoursePreference preference = preferenceRepository.findById(preferenceId)
                .orElseThrow(() -> new IllegalArgumentException("Preference not found: " + preferenceId));

        preference.setPercentage(dto.getPercentage());
        preference.validate();

        InstructorCoursePreference updated = preferenceRepository.save(preference);
        return mapToDTO(updated);
    }

    /**
     * Delete a preference
     */
    public void deletePreference(Long preferenceId) {
        log.info("Deleting preference: {}", preferenceId);
        if (!preferenceRepository.existsById(preferenceId)) {
            throw new IllegalArgumentException("Preference not found: " + preferenceId);
        }
        preferenceRepository.deleteById(preferenceId);
    }

    /**
     * Delete all preferences for a course
     */
    public void deleteAllPreferencesForCourse(Long courseId) {
        log.info("Deleting all preferences for course: {}", courseId);
        List<InstructorCoursePreference> preferences = preferenceRepository.findByOptionalCourseId(courseId);
        preferenceRepository.deleteAll(preferences);
    }

    private InstructorCoursePreferenceDTO mapToDTO(InstructorCoursePreference preference) {
        return InstructorCoursePreferenceDTO.builder()
                .optionalCourseId(preference.getOptionalCourse().getId())
                .optionalCourseCode(preference.getOptionalCourse().getCode())
                .compulsoryCourseCode(preference.getCompulsoryCourseCode())
                .percentage(preference.getPercentage())
                .build();
    }
}
