package uaic.dbxdrgsl.PrefSchedule.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import uaic.dbxdrgsl.PrefSchedule.model.Enrollment;

public interface EnrollmentRepository extends JpaRepository<Enrollment, Long> {

}
