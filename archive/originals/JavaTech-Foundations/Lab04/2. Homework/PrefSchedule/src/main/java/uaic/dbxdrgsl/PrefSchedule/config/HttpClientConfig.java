package uaic.dbxdrgsl.PrefSchedule.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.reactive.function.client.WebClient;

/**
 * Configuration for HTTP clients used in the application
 */
@Configuration
public class HttpClientConfig {

    /**
     * Create a WebClient bean for making HTTP requests to external services
     */
    @Bean
    public WebClient webClient(WebClient.Builder builder) {
        return builder.build();
    }
}
