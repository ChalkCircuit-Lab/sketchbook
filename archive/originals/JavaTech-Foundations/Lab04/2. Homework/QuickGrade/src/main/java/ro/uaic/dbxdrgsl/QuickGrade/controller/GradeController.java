package ro.uaic.dbxdrgsl.QuickGrade.controller;

import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import ro.uaic.dbxdrgsl.QuickGrade.model.GradeEvent;
import ro.uaic.dbxdrgsl.QuickGrade.service.GradeEventPublisher;

@RestController
@RequestMapping("/api/grades")
@RequiredArgsConstructor
public class GradeController {

    private final GradeEventPublisher publisher;

    @PostMapping
    public ResponseEntity<Void> publish(@RequestBody GradeEvent event) {
        publisher.publish(event);
        return ResponseEntity.status(HttpStatus.ACCEPTED).build();
    }

    @PostMapping("/demo")
    public ResponseEntity<Void> publishDemo() {
        GradeEvent demo = new GradeEvent("S001", "CS101", 9.5);
        publisher.publish(demo);
        return ResponseEntity.status(HttpStatus.ACCEPTED).build();
    }
}
