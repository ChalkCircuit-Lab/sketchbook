package uaic.dbxdrgsl.PrefSchedule.service;

import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import uaic.dbxdrgsl.PrefSchedule.model.Course;
import uaic.dbxdrgsl.PrefSchedule.model.Grade;
import uaic.dbxdrgsl.PrefSchedule.repository.CourseRepository;
import uaic.dbxdrgsl.PrefSchedule.repository.GradeRepository;

import java.util.List;

@Service
@RequiredArgsConstructor
public class GradeService {

    private final GradeRepository gradeRepository;
    private final CourseRepository courseRepository;

    public boolean isCourseCompulsory(String courseCode) {
        if (courseCode == null || courseCode.isBlank()) return false;
        List<Course> courses = courseRepository.findAll();
        return courses.stream().anyMatch(c -> courseCode.equalsIgnoreCase(c.getCode()) && c.isCompulsory());
    }

    @Transactional
    public Grade save(Grade g) {
        return gradeRepository.save(g);
    }

    public List<Grade> findAll() {
        return gradeRepository.findAll();
    }

    public List<Grade> findByStudent(String studentCode) {
        return gradeRepository.findByStudentCode(studentCode);
    }

    public List<Grade> findByCourse(String courseCode) {
        return gradeRepository.findByCourseCode(courseCode);
    }

    public List<Grade> findByStudentAndCourse(String studentCode, String courseCode) {
        return gradeRepository.findByStudentCodeAndCourseCode(studentCode, courseCode);
    }
}
