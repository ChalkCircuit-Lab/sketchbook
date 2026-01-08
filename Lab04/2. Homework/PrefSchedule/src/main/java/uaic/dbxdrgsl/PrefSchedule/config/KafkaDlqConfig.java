package uaic.dbxdrgsl.PrefSchedule.config;

import org.apache.kafka.clients.admin.NewTopic;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class KafkaDlqConfig {

    public static final String GRADES_TOPIC = "grades";
    public static final String DLT_TOPIC = "grades.DLT";

    @Bean
    public NewTopic gradesTopic() {
        return new NewTopic(GRADES_TOPIC, 3, (short) 1);
    }

    @Bean
    public NewTopic gradesDltTopic() {
        return new NewTopic(DLT_TOPIC, 3, (short) 1);
    }

}
