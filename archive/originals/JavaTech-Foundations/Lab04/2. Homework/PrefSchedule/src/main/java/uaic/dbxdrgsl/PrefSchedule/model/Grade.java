package uaic.dbxdrgsl.PrefSchedule.model;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.time.Instant;

@Entity
@Table(name = "grades")
@Getter
@Setter
@NoArgsConstructor
public class Grade {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String studentCode;

    @Column(nullable = false)
    private String courseCode;

    @Column(nullable = false)
    private double grade;

    @Column(nullable = false)
    private Instant receivedAt = Instant.now();
}
