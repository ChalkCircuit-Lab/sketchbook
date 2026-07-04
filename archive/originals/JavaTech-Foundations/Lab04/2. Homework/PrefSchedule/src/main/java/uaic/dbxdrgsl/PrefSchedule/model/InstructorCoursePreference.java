package uaic.dbxdrgsl.PrefSchedule.model;

import jakarta.persistence.*;
import lombok.*;

/**
 * Entity representing instructor preferences for students in optional courses.
 * Stores the weighted importance of each compulsory course for an optional course.
 * Example: For optional course "DataScience", Math has 70% importance, OOP has 30%.
 */
@Entity
@Table(name = "instructor_course_preferences", uniqueConstraints = {
    @UniqueConstraint(columnNames = {"optional_course_id", "compulsory_course_code"})
})
@NoArgsConstructor
@AllArgsConstructor
@Getter
@Setter
@Builder
@EqualsAndHashCode(exclude = {"optionalCourse"})
@ToString(exclude = {"optionalCourse"})
public class InstructorCoursePreference {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    /**
     * The optional course for which this preference is specified
     */
    @ManyToOne(fetch = FetchType.EAGER)
    @JoinColumn(name = "optional_course_id", nullable = false)
    private Course optionalCourse;

    /**
     * The code of the compulsory course that is important for the optional course
     * Example: "Math", "OOP", "Java"
     */
    @Column(nullable = false)
    private String compulsoryCourseCode;

    /**
     * The weight/percentage importance of this compulsory course (0-100)
     * Example: 70 means 70% importance
     */
    @Column(nullable = false)
    private double percentage;

    /**
     * Validates that percentage is between 0 and 100
     */
    @PrePersist
    @PreUpdate
    public void validate() {
        if (percentage < 0 || percentage > 100) {
            throw new IllegalArgumentException("Percentage must be between 0 and 100");
        }
    }
}
