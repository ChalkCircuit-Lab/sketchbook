package uaic.dbxdrgsl.PrefSchedule.controller;

import lombok.RequiredArgsConstructor;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;
import uaic.dbxdrgsl.PrefSchedule.model.Grade;
import uaic.dbxdrgsl.PrefSchedule.service.GradeService;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.*;

@RestController
@RequestMapping("/api/grades")
@RequiredArgsConstructor
public class GradesController {

    private final GradeService gradeService;

    @GetMapping
    public ResponseEntity<List<Grade>> getGrades(@RequestParam(required = false) String studentCode,
                                                 @RequestParam(required = false) String courseCode) {
        if (studentCode != null && courseCode != null) {
            return ResponseEntity.ok(gradeService.findByStudentAndCourse(studentCode, courseCode));
        } else if (studentCode != null) {
            return ResponseEntity.ok(gradeService.findByStudent(studentCode));
        } else if (courseCode != null) {
            return ResponseEntity.ok(gradeService.findByCourse(courseCode));
        } else {
            return ResponseEntity.ok(gradeService.findAll());
        }
    }

    @PostMapping(path = "/load-csv")
    public ResponseEntity<Map<String, Object>> loadCsv(@RequestParam("file") MultipartFile file) throws IOException {
        int total = 0, stored = 0, ignored = 0, failed = 0;
        List<String> failures = new ArrayList<>();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(file.getInputStream(), StandardCharsets.UTF_8))) {
            String line;
            while ((line = br.readLine()) != null) {
                total++;
                String[] parts = Arrays.stream(line.split(",")).map(String::trim).toArray(String[]::new);
                if (parts.length != 3) { failed++; failures.add("Invalid columns at line " + total); continue; }
                String student = parts[0];
                String course = parts[1];
                double gradeVal;
                try { gradeVal = Double.parseDouble(parts[2]); } catch (Exception e) { failed++; failures.add("Invalid grade at line " + total); continue; }
                if (student.isBlank() || course.isBlank()) { failed++; failures.add("Missing fields at line " + total); continue; }
                if (gradeService.isCourseCompulsory(course)) {
                    Grade g = new Grade();
                    g.setStudentCode(student);
                    g.setCourseCode(course);
                    g.setGrade(gradeVal);
                    gradeService.save(g);
                    stored++;
                } else {
                    ignored++;
                }
            }
        }
        Map<String, Object> resp = new LinkedHashMap<>();
        resp.put("totalRows", total);
        resp.put("storedRows", stored);
        resp.put("ignoredRows", ignored);
        resp.put("failedRows", failed);
        resp.put("failures", failures.size() > 10 ? failures.subList(0, 10) : failures);
        return ResponseEntity.ok(resp);
    }
}
