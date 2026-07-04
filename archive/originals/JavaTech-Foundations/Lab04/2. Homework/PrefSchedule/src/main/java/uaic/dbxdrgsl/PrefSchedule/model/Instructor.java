package uaic.dbxdrgsl.PrefSchedule.model;

import jakarta.persistence.*;
import lombok.*;
import com.fasterxml.jackson.annotation.JsonManagedReference;

import java.util.ArrayList;
import java.util.List;

@Entity
@Table(name = "instructors")
@NoArgsConstructor
@AllArgsConstructor
@Getter
@Setter
@Builder
@EqualsAndHashCode(exclude = {"user", "courses"})
@ToString(exclude = {"user", "courses"})
public class Instructor {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @OneToOne
    @JoinColumn(name = "user_id", nullable = false, unique = true)
    private User user;

    @Column(nullable = false)
    private String department;

    private String specialization;

    @OneToMany(mappedBy = "instructor", cascade = CascadeType.ALL, orphanRemoval = true)
    @JsonManagedReference("instr-courses")
    private List<Course> courses = new ArrayList<>();

    public void addCourse(Course c) {
        courses.add(c);
        c.setInstructor(this);
    }

    public void removeCourse(Course c) {
        courses.remove(c);
        c.setInstructor(null);
    }

}
