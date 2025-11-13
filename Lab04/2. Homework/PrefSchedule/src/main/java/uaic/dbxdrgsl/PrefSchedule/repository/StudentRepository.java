package uaic.dbxdrgsl.PrefSchedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import uaic.dbxdrgsl.PrefSchedule.model.Student;

import java.util.List;

public interface StudentRepository extends JpaRepository<Student, Long> {

    // derived query
    List<Student> findByLastName(String lastName);

}
