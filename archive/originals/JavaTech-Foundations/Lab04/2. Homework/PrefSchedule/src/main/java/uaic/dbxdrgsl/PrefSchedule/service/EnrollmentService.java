package uaic.dbxdrgsl.PrefSchedule.service;

import org.springframework.stereotype.Service;
import uaic.dbxdrgsl.PrefSchedule.model.Enrollment;
import uaic.dbxdrgsl.PrefSchedule.repository.EnrollmentRepository;

import java.util.List;
import java.util.Optional;

@Service
public class EnrollmentService {

    private final EnrollmentRepository repo;

    public EnrollmentService(EnrollmentRepository repo) {
        this.repo = repo;
    }

    public Enrollment save(Enrollment e) { return repo.save(e); }

    public List<Enrollment> findAll() { return repo.findAll(); }

    public Optional<Enrollment> findById(Long id) { return repo.findById(id); }

    public void deleteById(Long id) { repo.deleteById(id); }

}
