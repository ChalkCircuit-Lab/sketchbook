package uaic.dbxdrgsl.PrefSchedule.model;

import jakarta.persistence.Entity;
import jakarta.persistence.FetchType;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.ManyToOne;
import com.fasterxml.jackson.annotation.JsonBackReference;
import jakarta.persistence.Table;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import lombok.ToString;

@Entity
@Table(name = "enrollments")
@NoArgsConstructor
@Getter
@Setter
@ToString
public class Enrollment {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JsonBackReference("student-enrollments")
    private Student student;

    @ManyToOne(fetch = FetchType.LAZY)
    @JsonBackReference("course-enrollments")
    private Course course;

    // e.g. priority or preference ranking
    private Integer ranking;

}
