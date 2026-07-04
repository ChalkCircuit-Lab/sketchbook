package ro.uaic.dbxdrgsl.springboot_demo.boot;

import org.springframework.boot.CommandLineRunner;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Component;
import ro.uaic.dbxdrgsl.springboot_demo.repository.StudentRepository;

@Component
public class DatabasePrinter implements CommandLineRunner {
    private final StudentRepository repo;
    private final Environment env;

    public DatabasePrinter(StudentRepository repo, Environment env) {
        this.repo = repo; this.env = env;
    }

    @Override
    public void run(String... args) {
        System.out.println("Active profiles: " + String.join(",", env.getActiveProfiles()));
        System.out.println("DB URL: " + env.getProperty("spring.datasource.url","built programmatically"));
        System.out.println("Rows in students:");
        try { repo.findAll().forEach(System.out::println); }
        catch (Exception e) { System.out.println("Query failed: " + e.getMessage()); }
    }
}
