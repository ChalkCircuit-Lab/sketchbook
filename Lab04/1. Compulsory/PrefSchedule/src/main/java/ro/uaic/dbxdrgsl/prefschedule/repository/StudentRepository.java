package ro.uaic.dbxdrgsl.prefschedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import ro.uaic.dbxdrgsl.prefschedule.model.Student;

public interface StudentRepository extends JpaRepository<Student, Long> {
}
