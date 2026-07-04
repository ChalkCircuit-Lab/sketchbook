package uaic.dbxdrgsl.PrefSchedule.service;

import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import uaic.dbxdrgsl.PrefSchedule.model.Course;
import uaic.dbxdrgsl.PrefSchedule.repository.CourseRepository;

import java.util.List;
import java.util.Optional;

@Service
public class CourseService {

    private final CourseRepository repo;

    public CourseService(CourseRepository repo) {
        this.repo = repo;
    }

    public Course save(Course c) {
        return repo.save(c);
    }

    public List<Course> findAll() {
        return repo.findAll();
    }

    public Optional<Course> findById(Long id) {
        return repo.findById(id);
    }

    public void deleteById(Long id) {
        repo.deleteById(id);
    }

    public List<Course> searchByTitle(String fragment) {
        return repo.findByTitleContainingIgnoreCase(fragment);
    }

    public List<Course> findByInstructorLastName(String lastName) {
        return repo.findByInstructorLastName(lastName);
    }

    @Transactional
    public boolean updateCredits(Long id, int credits) {
        int updated = repo.updateCreditsById(id, credits);
        return updated > 0;
    }

}
