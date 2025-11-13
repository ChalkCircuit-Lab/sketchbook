package uaic.dbxdrgsl.PrefSchedule.model;

import jakarta.persistence.CascadeType;
import jakarta.persistence.Entity;
import jakarta.persistence.OneToMany;
import jakarta.persistence.Table;
import lombok.EqualsAndHashCode;
import lombok.NoArgsConstructor;
import lombok.ToString;
import lombok.Getter;
import lombok.Setter;
import com.fasterxml.jackson.annotation.JsonManagedReference;

import java.util.ArrayList;
import java.util.List;

@Entity
@Table(name = "students")
@NoArgsConstructor
@Getter
@Setter
@EqualsAndHashCode(callSuper = true)
@ToString(callSuper = true)
public class Student extends Person {

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
