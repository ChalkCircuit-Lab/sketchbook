package uaic.dbxdrgsl.PrefSchedule.messaging;

import lombok.extern.slf4j.Slf4j;
import org.apache.kafka.clients.consumer.ConsumerRecord;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.messaging.handler.annotation.Header;
import org.springframework.messaging.handler.annotation.Payload;
import org.springframework.kafka.support.KafkaHeaders;
import org.springframework.stereotype.Component;

@Slf4j
@Component
public class GradesDltListener {

    @KafkaListener(topics = "grades.DLT", groupId = "prefschedule-grades-dlt")
    public void onDlt(ConsumerRecord<String, Object> record,
                      @Header(KafkaHeaders.DLT_EXCEPTION_FQCN) String exceptionClass,
                      @Header(KafkaHeaders.DLT_EXCEPTION_MESSAGE) String exceptionMessage,
                      @Header(KafkaHeaders.RECEIVED_TIMESTAMP) Long ts) {
        log.warn("DLT received: key={}, value={}, exception={}, message={}, ts={}",
                record.key(), record.value(), exceptionClass, exceptionMessage, ts);
    }
}
