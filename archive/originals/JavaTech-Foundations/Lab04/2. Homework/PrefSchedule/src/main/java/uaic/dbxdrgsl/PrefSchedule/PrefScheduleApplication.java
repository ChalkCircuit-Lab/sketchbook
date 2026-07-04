package uaic.dbxdrgsl.PrefSchedule;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.domain.EntityScan;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;

@SpringBootApplication
@EntityScan(basePackages = "uaic.dbxdrgsl.PrefSchedule.model")
@EnableJpaRepositories(basePackages = "uaic.dbxdrgsl.PrefSchedule.repository")
public class PrefScheduleApplication {

	public static void main(String[] args) {
		SpringApplication.run(PrefScheduleApplication.class, args);
	}

}
