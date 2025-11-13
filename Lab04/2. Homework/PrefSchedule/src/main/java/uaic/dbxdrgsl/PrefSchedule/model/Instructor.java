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
@Table(name = "instructors")
@NoArgsConstructor
@Getter
@Setter
@EqualsAndHashCode(callSuper = true)
@ToString(callSuper = true)
public class Instructor extends Person {

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
