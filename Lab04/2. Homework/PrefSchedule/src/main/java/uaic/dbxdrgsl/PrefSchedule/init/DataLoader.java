package uaic.dbxdrgsl.PrefSchedule.init;

import net.datafaker.Faker;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;
import uaic.dbxdrgsl.PrefSchedule.model.Course;
import uaic.dbxdrgsl.PrefSchedule.model.Enrollment;
import uaic.dbxdrgsl.PrefSchedule.model.Instructor;
import uaic.dbxdrgsl.PrefSchedule.model.Student;
import uaic.dbxdrgsl.PrefSchedule.service.CourseService;
import uaic.dbxdrgsl.PrefSchedule.service.EnrollmentService;
import uaic.dbxdrgsl.PrefSchedule.service.InstructorService;
import uaic.dbxdrgsl.PrefSchedule.service.StudentService;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

@Component
public class DataLoader implements CommandLineRunner {

    private final StudentService studentService;
    private final InstructorService instructorService;
    private final CourseService courseService;
    private final EnrollmentService enrollmentService;

    public DataLoader(StudentService studentService, InstructorService instructorService, CourseService courseService, EnrollmentService enrollmentService) {
        this.studentService = studentService;
        this.instructorService = instructorService;
        this.courseService = courseService;
        this.enrollmentService = enrollmentService;
    }

    @Override
    public void run(String... args) throws Exception {
        Faker faker = new Faker();
        Random rnd = new Random();

        // create instructors
        List<Instructor> instructors = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            Instructor inst = new Instructor();
            inst.setFirstName(faker.name().firstName());
            inst.setLastName(faker.name().lastName());
            inst.setEmail(inst.getFirstName().toLowerCase() + "." + inst.getLastName().toLowerCase() + "@uni.edu");
            instructors.add(instructorService.save(inst));
        }

        // create students
        List<Student> students = new ArrayList<>();
        for (int i = 0; i < 20; i++) {
            Student s = new Student();
            s.setFirstName(faker.name().firstName());
            s.setLastName(faker.name().lastName());
            s.setEmail(s.getFirstName().toLowerCase() + "." + s.getLastName().toLowerCase() + "@student.uni.edu");
            students.add(studentService.save(s));
        }

        // create courses
        List<Course> courses = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            Course c = new Course();
            c.setCode("C" + (100 + i));
            c.setTitle(faker.educator().course());
            c.setCredits(3 + rnd.nextInt(3));
            // assign random instructor
            Instructor inst = instructors.get(rnd.nextInt(instructors.size()));
            c.setInstructor(inst);
            courses.add(courseService.save(c));
        }

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
