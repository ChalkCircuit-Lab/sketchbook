package uaic.dbxdrgsl.PrefSchedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import uaic.dbxdrgsl.PrefSchedule.model.Student;

import java.util.List;

public interface StudentRepository extends JpaRepository<Student, Long> {

    // derived query - accessing user's lastName through the relationship
    List<Student> findByUserLastName(String lastName);

}
