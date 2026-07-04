package uaic.dbxdrgsl.PrefSchedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.transaction.annotation.Transactional;
import uaic.dbxdrgsl.PrefSchedule.model.Course;

import java.util.List;

public interface CourseRepository extends JpaRepository<Course, Long> {

    // derived query
    List<Course> findByTitleContainingIgnoreCase(String fragment);

    // JPQL query - find courses by instructor last name (accessing through user relationship)
    @Query("SELECT c FROM Course c WHERE c.instructor.user.lastName = :lastName")
    List<Course> findByInstructorLastName(@Param("lastName") String lastName);

    // modifying, transactional query to update credits
    @Modifying
    @Transactional
    @Query("UPDATE Course c SET c.credits = :credits WHERE c.id = :id")
    int updateCreditsById(@Param("id") Long id, @Param("credits") int credits);

}
