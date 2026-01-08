package uaic.dbxdrgsl.PrefSchedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;
import uaic.dbxdrgsl.PrefSchedule.model.InstructorCoursePreference;

import java.util.List;

@Repository
public interface InstructorCoursePreferenceRepository extends JpaRepository<InstructorCoursePreference, Long> {

    /**
     * Find all preferences for a specific optional course
     */
    List<InstructorCoursePreference> findByOptionalCourseId(Long courseId);

    /**
     * Find all preferences for a specific optional course code
     */
    @Query("SELECT icp FROM InstructorCoursePreference icp WHERE icp.optionalCourse.code = :courseCode")
    List<InstructorCoursePreference> findByOptionalCourseCode(@Param("courseCode") String courseCode);

    /**
     * Check if a preference exists for a course and compulsory course pair
     */
    boolean existsByOptionalCourseIdAndCompulsoryCourseCode(Long optionalCourseId, String compulsoryCourseCode);
}
