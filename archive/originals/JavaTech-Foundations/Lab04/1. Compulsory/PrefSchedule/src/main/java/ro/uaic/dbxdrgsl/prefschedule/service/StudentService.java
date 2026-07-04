package ro.uaic.dbxdrgsl.prefschedule.service;

import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.prefschedule.model.Student;
import ro.uaic.dbxdrgsl.prefschedule.repository.StudentRepository;

import java.util.List;

@Service
@RequiredArgsConstructor
public class StudentService {
    private final StudentRepository repo;

    public List<Student> findAll() {
        return repo.findAll();
    }

    public Student findById(Long id) {
        return repo.findById(id).orElseThrow(() -> new RuntimeException("Student not found"));
    }

    public Student create(Student student) {
        return repo.save(student);
    }

    public Student update(Long id, Student newData) {
        Student s = findById(id);
        s.setCode(newData.getCode());
        s.setName(newData.getName());
        s.setEmail(newData.getEmail());
        s.setYear(newData.getYear());
        return repo.save(s);
    }

    public void delete(Long id) {
        repo.deleteById(id);
    }
}
