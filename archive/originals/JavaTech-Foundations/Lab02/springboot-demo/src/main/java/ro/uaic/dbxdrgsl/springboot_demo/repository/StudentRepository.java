package ro.uaic.dbxdrgsl.springboot_demo.repository;

import java.util.List;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Repository;
import ro.uaic.dbxdrgsl.springboot_demo.model.Student;

@Repository
public class StudentRepository {
    private final JdbcTemplate jdbc;
    public StudentRepository(JdbcTemplate jdbc){ this.jdbc = jdbc; }

    public List<Student> findAll() {
        return jdbc.query("SELECT id, name FROM students",
                (rs, i) -> new Student(rs.getInt("id"), rs.getString("name")));
    }
}
