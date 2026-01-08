package uaic.dbxdrgsl.PrefSchedule.init;

import net.datafaker.Faker;
import jakarta.persistence.EntityManager;
import org.springframework.boot.context.event.ApplicationReadyEvent;
import org.springframework.context.annotation.DependsOn;
import org.springframework.context.event.EventListener;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.stereotype.Component;
import org.springframework.transaction.annotation.Transactional;
import uaic.dbxdrgsl.PrefSchedule.model.*;
import uaic.dbxdrgsl.PrefSchedule.repository.InstructorRepository;
import uaic.dbxdrgsl.PrefSchedule.repository.StudentRepository;
import uaic.dbxdrgsl.PrefSchedule.repository.UserRepository;
import uaic.dbxdrgsl.PrefSchedule.service.CourseService;
import uaic.dbxdrgsl.PrefSchedule.service.EnrollmentService;
import uaic.dbxdrgsl.PrefSchedule.service.InstructorService;
import uaic.dbxdrgsl.PrefSchedule.service.StudentService;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

@Component
@ConditionalOnProperty(name = "app.data-loader.enabled", havingValue = "true", matchIfMissing = true)
@DependsOn("entityManagerFactory")
public class DataLoader {

    private final StudentService studentService;
    private final InstructorService instructorService;
    private final CourseService courseService;
    private final EnrollmentService enrollmentService;
    private final UserRepository userRepository;
    private final StudentRepository studentRepository;
    private final InstructorRepository instructorRepository;
    private final PasswordEncoder passwordEncoder;
    private final EntityManager entityManager;

    public DataLoader(StudentService studentService, InstructorService instructorService, CourseService courseService,
                     EnrollmentService enrollmentService, UserRepository userRepository, StudentRepository studentRepository,
                     InstructorRepository instructorRepository, PasswordEncoder passwordEncoder, EntityManager entityManager) {
        this.studentService = studentService;
        this.instructorService = instructorService;
        this.courseService = courseService;
        this.enrollmentService = enrollmentService;
        this.userRepository = userRepository;
        this.studentRepository = studentRepository;
        this.instructorRepository = instructorRepository;
        this.passwordEncoder = passwordEncoder;
        this.entityManager = entityManager;
    }

    @EventListener(ApplicationReadyEvent.class)
    @Transactional
    public void loadData() {
        Faker faker = new Faker();
        Random rnd = new Random();

        // Create default users if they don't exist
        if (!userRepository.existsByUsername("admin")) {
            User adminUser = User.builder()
                .username("admin")
                .password(passwordEncoder.encode("admin123"))
                .email("admin@uni.edu")
                .firstName("Admin")
                .lastName("User")
                .role(UserRole.ADMIN)
                .enabled(true)
                .build();
            userRepository.save(adminUser);
            System.out.println("Created ADMIN user: admin");
        }

        // Create instructors with users
        List<Instructor> instructors = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            String firstName = faker.name().firstName();
            String lastName = faker.name().lastName();
            String username = firstName.toLowerCase() + "_inst" + i;

            if (!userRepository.existsByUsername(username)) {
                User instructorUser = User.builder()
                    .username(username)
                    .password(passwordEncoder.encode("password123"))
                    .email(username + "@uni.edu")
                    .firstName(firstName)
                    .lastName(lastName)
                    .role(UserRole.INSTRUCTOR)
                    .enabled(true)
                    .build();
                instructorUser = userRepository.save(instructorUser);

                Instructor inst = Instructor.builder()
                    .user(instructorUser)
                    .department("Computer Science")
                    .specialization("Software Engineering")
                    .build();
                instructors.add(instructorRepository.save(inst));
                System.out.println("Created INSTRUCTOR user: " + username);
            } else {
                instructors.add(instructorRepository.findAll().get(i % instructorRepository.findAll().size()));
            }
        }

        // Create students with users
        List<Student> students = new ArrayList<>();
        for (int i = 0; i < 20; i++) {
            String firstName = faker.name().firstName();
            String lastName = faker.name().lastName();
            String username = firstName.toLowerCase() + "_std" + i;

            if (!userRepository.existsByUsername(username)) {
                User studentUser = User.builder()
                    .username(username)
                    .password(passwordEncoder.encode("password123"))
                    .email(username + "@student.uni.edu")
                    .firstName(firstName)
                    .lastName(lastName)
                    .role(UserRole.STUDENT)
                    .enabled(true)
                    .build();
                studentUser = userRepository.save(studentUser);

                Student s = Student.builder()
                    .user(studentUser)
                    .studentNumber("STU" + String.format("%05d", 10000 + i))
                    .group("Group" + (i % 4 + 1))
                    .build();
                students.add(studentRepository.save(s));
                System.out.println("Created STUDENT user: " + username);
            } else {
                students.add(studentRepository.findAll().get(i % studentRepository.findAll().size()));
            }
        }

        // create courses (mark some as compulsory) + a few known codes for testing
        List<Course> courses = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            Course c = new Course();
            c.setCode("C" + (100 + i));
            c.setTitle(faker.educator().course());
            c.setCredits(3 + rnd.nextInt(3));
            // mark half as compulsory
            c.setCompulsory(i % 2 == 0);
            // assign random instructor
            Instructor inst = instructors.get(rnd.nextInt(instructors.size()));
            c.setInstructor(inst);
            courses.add(courseService.save(c));
        }

        // Add specific demo courses
        Course cs101 = new Course();
        cs101.setCode("CS101");
        cs101.setTitle("Intro to Computer Science");
        cs101.setCredits(5);
        cs101.setCompulsory(true);
        cs101.setInstructor(instructors.get(0));
        courses.add(courseService.save(cs101));

        Course cs102 = new Course();
        cs102.setCode("CS102");
        cs102.setTitle("Data Structures");
        cs102.setCredits(5);
        cs102.setCompulsory(true);
        cs102.setInstructor(instructors.get(1 % instructors.size()));
        courses.add(courseService.save(cs102));

        Course el201 = new Course();
        el201.setCode("EL201");
        el201.setTitle("Elective: Creative Coding");
        el201.setCredits(4);
        el201.setCompulsory(false);
        el201.setInstructor(instructors.get(2 % instructors.size()));
        courses.add(courseService.save(el201));

        // enroll some students randomly
        for (Student s : students) {
            int enrollCount = 1 + rnd.nextInt(4);
            for (int j = 0; j < enrollCount; j++) {
                Course c = courses.get(rnd.nextInt(courses.size()));
                Enrollment e = new Enrollment();
                e.setCourse(c);
                e.setStudent(s);
                e.setRanking(1 + rnd.nextInt(5));
                enrollmentService.save(e);
            }
        }

        // CRUD test for courses
        System.out.println("=== COURSES BEFORE ===");
        courseService.findAll().forEach(co -> System.out.println(co.getId() + " - " + co.getTitle() + " (" + co.getCredits() + ")"));

        // create new course
        Course newCourse = new Course();
        newCourse.setCode("C999");
        newCourse.setTitle("Intro to Preferences");
        newCourse.setCredits(5);
        newCourse.setCompulsory(true);
        newCourse.setInstructor(instructors.get(0));
        newCourse = courseService.save(newCourse);
        System.out.println("Created course: " + newCourse.getId());

        // update credits via modifying query
        boolean updated = courseService.updateCredits(newCourse.getId(), 4);
        System.out.println("Updated credits? " + updated);

        // read course
        courseService.findById(newCourse.getId()).ifPresent(c -> System.out.println("Read course after update: " + c.getTitle() + " credits=" + c.getCredits()));

        // delete course
        courseService.deleteById(newCourse.getId());
        System.out.println("Deleted course id=" + newCourse.getId());

        System.out.println("=== COURSES AFTER ===");
        courseService.findAll().forEach(co -> System.out.println(co.getId() + " - " + co.getTitle() + " (" + co.getCredits() + ")"));

    }
}
