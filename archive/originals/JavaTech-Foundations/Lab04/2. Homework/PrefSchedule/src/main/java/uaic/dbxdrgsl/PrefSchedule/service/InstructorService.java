package uaic.dbxdrgsl.PrefSchedule.service;

import org.springframework.stereotype.Service;
import uaic.dbxdrgsl.PrefSchedule.model.Instructor;
import uaic.dbxdrgsl.PrefSchedule.repository.InstructorRepository;

import java.util.List;
import java.util.Optional;

@Service
public class InstructorService {

    private final InstructorRepository repo;

    public InstructorService(InstructorRepository repo) {
        this.repo = repo;
    }

    public Instructor save(Instructor i) { return repo.save(i); }

    public List<Instructor> findAll() { return repo.findAll(); }

    public Optional<Instructor> findById(Long id) { return repo.findById(id); }

    public void deleteById(Long id) { repo.deleteById(id); }

}
