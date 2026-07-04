package uaic.dbxdrgsl.PrefSchedule.messaging;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.stereotype.Component;
import uaic.dbxdrgsl.PrefSchedule.model.Grade;
import uaic.dbxdrgsl.PrefSchedule.service.GradeService;

@Slf4j
@Component
@RequiredArgsConstructor
public class GradesListener {

    private final GradeService gradeService;

    @KafkaListener(topics = "grades", groupId = "prefschedule-grades")
    public void onGrade(GradeEvent event) {
        log.info("Received grade: studentCode={}, courseCode={}, grade={}", event.studentCode(), event.courseCode(), event.grade());
        if (event.studentCode() == null || event.studentCode().isBlank() ||
                event.courseCode() == null || event.courseCode().isBlank() ||
                event.grade() < 1.0 || event.grade() > 10.0) {
            throw new IllegalArgumentException("Invalid grade event payload");
        }
        if (gradeService.isCourseCompulsory(event.courseCode())) {
            Grade g = new Grade();
            g.setStudentCode(event.studentCode());
            g.setCourseCode(event.courseCode());
            g.setGrade(event.grade());
            gradeService.save(g);
            log.info("Stored grade for compulsory course {}", event.courseCode());
        } else {
            log.info("Ignored grade for non-compulsory course {}", event.courseCode());
        }
    }
}
