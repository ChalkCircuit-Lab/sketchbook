package ro.uaic.dbxdrgsl.springboot_demo.config;

import javax.sql.DataSource;
import org.springframework.boot.autoconfigure.condition.ConditionalOnExpression;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.annotation.*;
import org.springframework.jdbc.core.JdbcTemplate;

@Configuration
@EnableConfigurationProperties(AppDbProperties.class)
public class DataSourceConfig {

    @Bean
    @Profile("dev")
    @ConditionalOnExpression("'${app.db.vendor}'.toLowerCase().contains('h2')")
    public DataSource devDataSource(AppDbProperties p) {
        String url = "jdbc:h2:mem:" + p.getName();
        return org.springframework.boot.jdbc.DataSourceBuilder.create()
                .url(url).username(p.getUser()).password(p.getPassword())
                .driverClassName("org.h2.Driver").build();
    }

    @Bean
    @Profile("prod")
    @ConditionalOnExpression("'${app.db.vendor}'.toLowerCase().contains('mysql')")
    public DataSource prodDataSource(AppDbProperties p) {
        String url = "jdbc:mysql://" + p.getHost() + ":" + p.getPort() + "/" + p.getName() + "?useSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC";
        return org.springframework.boot.jdbc.DataSourceBuilder.create()
                .url(url).username(p.getUser()).password(p.getPassword())
                .driverClassName("com.mysql.cj.jdbc.Driver").build();
    }

    @Bean
    public JdbcTemplate jdbcTemplate(DataSource ds) {
        return new JdbcTemplate(ds);
    }
}
