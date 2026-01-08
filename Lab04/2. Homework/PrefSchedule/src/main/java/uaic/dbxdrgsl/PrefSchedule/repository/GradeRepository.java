package uaic.dbxdrgsl.PrefSchedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import uaic.dbxdrgsl.PrefSchedule.model.Grade;

import java.util.List;

public interface GradeRepository extends JpaRepository<Grade, Long> {
    List<Grade> findByStudentCode(String studentCode);
    List<Grade> findByCourseCode(String courseCode);
    List<Grade> findByStudentCodeAndCourseCode(String studentCode, String courseCode);
}
