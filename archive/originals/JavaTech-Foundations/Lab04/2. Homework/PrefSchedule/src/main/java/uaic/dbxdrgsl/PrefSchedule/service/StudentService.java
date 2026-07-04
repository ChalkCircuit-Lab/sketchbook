package uaic.dbxdrgsl.PrefSchedule.service;

import org.springframework.stereotype.Service;
import uaic.dbxdrgsl.PrefSchedule.model.Student;
import uaic.dbxdrgsl.PrefSchedule.repository.StudentRepository;

import java.util.List;
import java.util.Optional;

@Service
public class StudentService {

    private final StudentRepository repo;

    public StudentService(StudentRepository repo) {
        this.repo = repo;
    }

    public Student save(Student s) {
        return repo.save(s);
    }

    public List<Student> findAll() { return repo.findAll(); }

    public Optional<Student> findById(Long id) { return repo.findById(id); }

    public void deleteById(Long id) { repo.deleteById(id); }

    public List<Student> findByLastName(String lastName) { return repo.findByUserLastName(lastName); }

}
