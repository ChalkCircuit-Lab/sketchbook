package uaic.dbxdrgsl.PrefSchedule.controller;

import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.*;
import uaic.dbxdrgsl.PrefSchedule.model.Student;
import uaic.dbxdrgsl.PrefSchedule.service.StudentService;

import java.net.URI;
import java.util.List;

@RestController
@RequestMapping("/api/students")
public class StudentController {

    private final StudentService studentService;

    public StudentController(StudentService studentService) {
        this.studentService = studentService;
    }

    @GetMapping
    public List<Student> getAll() {
        return studentService.findAll();
    }

    @GetMapping("/{id}")
    public ResponseEntity<Student> getById(@PathVariable Long id) {
        return studentService.findById(id)
                .map(ResponseEntity::ok)
                .orElseGet(() -> ResponseEntity.notFound().build());
    }

    @PostMapping
    @PreAuthorize("hasRole('ADMIN')")
    public ResponseEntity<Student> create(@RequestBody Student student) {
        Student saved = studentService.save(student);
        // return 201 Created with location header
        return ResponseEntity.created(URI.create("/api/students/" + saved.getId())).body(saved);
    }

    @PutMapping("/{id}")
    @PreAuthorize("hasRole('ADMIN') or hasRole('INSTRUCTOR')")
    public ResponseEntity<Student> update(@PathVariable Long id, @RequestBody Student student) {
        return studentService.findById(id).map(existing -> {
            if (student.getUser() != null && student.getUser().getFirstName() != null) {
                existing.getUser().setFirstName(student.getUser().getFirstName());
            }
            if (student.getUser() != null && student.getUser().getLastName() != null) {
                existing.getUser().setLastName(student.getUser().getLastName());
            }
            if (student.getUser() != null && student.getUser().getEmail() != null) {
                existing.getUser().setEmail(student.getUser().getEmail());
            }
            if (student.getGroup() != null) {
                existing.setGroup(student.getGroup());
            }
            Student updated = studentService.save(existing);
            return ResponseEntity.ok(updated);
        }).orElseGet(() -> ResponseEntity.notFound().build());
    }

    @DeleteMapping("/{id}")
    @PreAuthorize("hasRole('ADMIN')")
    public ResponseEntity<Void> delete(@PathVariable Long id) {
        return studentService.findById(id).map(existing -> {
            studentService.deleteById(id);
            return new ResponseEntity<Void>(HttpStatus.NO_CONTENT);
        }).orElseGet(() -> ResponseEntity.notFound().build());
    }

}
