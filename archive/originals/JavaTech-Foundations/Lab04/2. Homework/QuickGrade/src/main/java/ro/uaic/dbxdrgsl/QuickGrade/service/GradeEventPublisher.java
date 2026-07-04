package ro.uaic.dbxdrgsl.QuickGrade.service;

import lombok.RequiredArgsConstructor;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.QuickGrade.config.KafkaConfig;
import ro.uaic.dbxdrgsl.QuickGrade.model.GradeEvent;

@Service
@RequiredArgsConstructor
public class GradeEventPublisher {

    private final KafkaTemplate<String, Object> kafkaTemplate;

    public void publish(GradeEvent event) {
        kafkaTemplate.send(KafkaConfig.GRADES_TOPIC, event.studentCode(), event);
    }
}
