package uaic.dbxdrgsl.PrefSchedule.model;

import jakarta.persistence.*;
import lombok.*;
import com.fasterxml.jackson.annotation.JsonManagedReference;

import java.util.ArrayList;
import java.util.List;

@Entity
@Table(name = "students")
@NoArgsConstructor
@AllArgsConstructor
@Getter
@Setter
@Builder
@EqualsAndHashCode(exclude = {"user", "enrollments"})
@ToString(exclude = {"user", "enrollments"})
public class Student {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @OneToOne
    @JoinColumn(name = "user_id", nullable = false, unique = true)
    private User user;

    @Column(nullable = false, unique = true)
    private String studentNumber;

    @Column(name = "`group`")
    private String group;

    @OneToMany(mappedBy = "student", cascade = CascadeType.ALL, orphanRemoval = true)
    @JsonManagedReference("student-enrollments")
    private List<Enrollment> enrollments = new ArrayList<>();

    public void addEnrollment(Enrollment e) {
        enrollments.add(e);
        e.setStudent(this);
    }

    public void removeEnrollment(Enrollment e) {
        enrollments.remove(e);
        e.setStudent(null);
    }
}
