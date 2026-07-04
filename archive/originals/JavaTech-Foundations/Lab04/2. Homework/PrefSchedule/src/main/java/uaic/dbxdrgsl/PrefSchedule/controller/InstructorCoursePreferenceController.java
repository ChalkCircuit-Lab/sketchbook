package uaic.dbxdrgsl.PrefSchedule.controller;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.*;
import uaic.dbxdrgsl.PrefSchedule.dto.InstructorCoursePreferenceDTO;
import uaic.dbxdrgsl.PrefSchedule.dto.InstructorCoursePreferenceBatchDTO;
import uaic.dbxdrgsl.PrefSchedule.service.InstructorCoursePreferenceService;

import java.util.List;

/**
 * REST Controller for managing instructor course preferences.
 * Instructors specify which compulsory courses and their importance for optional courses.
 */
@Slf4j
@RestController
@RequestMapping("/api/instructor-preferences")
@RequiredArgsConstructor
@CrossOrigin(origins = "*", maxAge = 3600)
public class InstructorCoursePreferenceController {

    private final InstructorCoursePreferenceService preferenceService;

    /**
     * Create a new instructor course preference
     */
    @PostMapping
    @PreAuthorize("hasAnyRole('INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<InstructorCoursePreferenceDTO> createPreference(
            @RequestBody InstructorCoursePreferenceDTO request) {
        log.info("POST /api/instructor-preferences - Creating preference");
        try {
            InstructorCoursePreferenceDTO result = preferenceService.createPreference(request);
            return ResponseEntity.status(HttpStatus.CREATED).body(result);
        } catch (IllegalArgumentException e) {
            log.error("Error creating preference: {}", e.getMessage());
            return ResponseEntity.badRequest().build();
        }
    }

    /**
     * Create multiple preferences for an optional course in batch
     */
    @PostMapping("/batch")
    @PreAuthorize("hasAnyRole('INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<List<InstructorCoursePreferenceDTO>> createBatchPreferences(
            @RequestBody InstructorCoursePreferenceBatchDTO request) {
        log.info("POST /api/instructor-preferences/batch - Creating {} preferences", request.getPreferences().size());
        try {
            List<InstructorCoursePreferenceDTO> result = preferenceService.createBatchPreferences(request);
            return ResponseEntity.status(HttpStatus.CREATED).body(result);
        } catch (IllegalArgumentException e) {
            log.error("Error creating batch preferences: {}", e.getMessage());
            return ResponseEntity.badRequest().build();
        }
    }

    /**
     * Get all preferences for a specific optional course
     */
    @GetMapping("/course/{courseId}")
    @PreAuthorize("hasAnyRole('STUDENT', 'INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<List<InstructorCoursePreferenceDTO>> getPreferencesForCourse(
            @PathVariable Long courseId) {
        log.info("GET /api/instructor-preferences/course/{} - Retrieving preferences", courseId);
        try {
            List<InstructorCoursePreferenceDTO> result = preferenceService.getPreferencesForCourse(courseId);
            return ResponseEntity.ok(result);
        } catch (IllegalArgumentException e) {
            log.error("Error retrieving preferences: {}", e.getMessage());
            return ResponseEntity.notFound().build();
        }
    }

    /**
     * Get all preferences for a specific optional course by code
     */
    @GetMapping("/course-code/{courseCode}")
    @PreAuthorize("hasAnyRole('STUDENT', 'INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<List<InstructorCoursePreferenceDTO>> getPreferencesForCourseCode(
            @PathVariable String courseCode) {
        log.info("GET /api/instructor-preferences/course-code/{} - Retrieving preferences", courseCode);
        List<InstructorCoursePreferenceDTO> result = preferenceService.getPreferencesForCourseCode(courseCode);
        return ResponseEntity.ok(result);
    }

    /**
     * Update an existing preference
     */
    @PutMapping("/{preferenceId}")
    @PreAuthorize("hasAnyRole('INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<InstructorCoursePreferenceDTO> updatePreference(
            @PathVariable Long preferenceId,
            @RequestBody InstructorCoursePreferenceDTO request) {
        log.info("PUT /api/instructor-preferences/{} - Updating preference", preferenceId);
        try {
            InstructorCoursePreferenceDTO result = preferenceService.updatePreference(preferenceId, request);
            return ResponseEntity.ok(result);
        } catch (IllegalArgumentException e) {
            log.error("Error updating preference: {}", e.getMessage());
            return ResponseEntity.notFound().build();
        }
    }

    /**
     * Delete a preference
     */
    @DeleteMapping("/{preferenceId}")
    @PreAuthorize("hasAnyRole('INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<Void> deletePreference(@PathVariable Long preferenceId) {
        log.info("DELETE /api/instructor-preferences/{} - Deleting preference", preferenceId);
        try {
            preferenceService.deletePreference(preferenceId);
            return ResponseEntity.noContent().build();
        } catch (IllegalArgumentException e) {
            log.error("Error deleting preference: {}", e.getMessage());
            return ResponseEntity.notFound().build();
        }
    }

    /**
     * Delete all preferences for a course
     */
    @DeleteMapping("/course/{courseId}/all")
    @PreAuthorize("hasAnyRole('INSTRUCTOR', 'ADMIN')")
    public ResponseEntity<Void> deleteAllPreferencesForCourse(@PathVariable Long courseId) {
        log.info("DELETE /api/instructor-preferences/course/{}/all - Deleting all preferences", courseId);
        preferenceService.deleteAllPreferencesForCourse(courseId);
        return ResponseEntity.noContent().build();
    }
}
