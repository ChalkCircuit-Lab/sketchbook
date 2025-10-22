package com.example.demo.config;

import com.zaxxer.hikari.HikariConfig;
import com.zaxxer.hikari.HikariDataSource;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.condition.ConditionalOnExpression;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.datasource.DataSourceTransactionManager;
import org.springframework.transaction.PlatformTransactionManager;

import javax.sql.DataSource;
import java.util.logging.Logger;

@Configuration
public class DataSourceConfig {

    private static final Logger logger = Logger.getLogger(DataSourceConfig.class.getName());

    @Value("${database.environment}")
    private String databaseEnvironment;

    @Value("${database.connection.host}")
    private String host;

    @Value("${database.connection.port}")
    private int port;

    @Value("${database.connection.name}")
    private String databaseName;

    @Value("${database.connection.user}")
    private String username;

    @Value("${database.connection.password}")
    private String password;

    /**
     * Development DataSource - H2 in-memory database
     * Uses SpEL to conditionally create this bean only for dev profile
     */
    @Bean
    @Profile("dev")
    @ConditionalOnExpression("'${database.environment}' == 'development'")
    public DataSource devDataSource() {
        logger.info("Creating Development DataSource (H2) for environment: " + databaseEnvironment);
        logger.info("Database configuration - Host: " + host + ", Port: " + port + ", Database: " + databaseName);

        HikariConfig config = new HikariConfig();
        config.setJdbcUrl("jdbc:h2:mem:" + databaseName + ";DB_CLOSE_DELAY=-1;DB_CLOSE_ON_EXIT=FALSE");
        config.setDriverClassName("org.h2.Driver");
        config.setUsername(username);
        config.setPassword(password);
        config.setConnectionTimeout(30000);
        config.setIdleTimeout(600000);
        config.setMaxLifetime(1800000);
        config.setMaximumPoolSize(10);
        config.setMinimumIdle(5);
        config.setPoolName("DevHikariPool");

        return new HikariDataSource(config);
    }

    /**
     * Production DataSource - PostgreSQL
     * Uses SpEL to conditionally create this bean only for prod profile
     */
    @Bean
    @Profile("prod")
    @ConditionalOnExpression("'${database.environment}' == 'production'")
    public DataSource prodDataSource() {
        logger.info("Creating Production DataSource (PostgreSQL) for environment: " + databaseEnvironment);
        logger.info("Database configuration - Host: " + host + ", Port: " + port + ", Database: " + databaseName);

        HikariConfig config = new HikariConfig();
        config.setJdbcUrl("jdbc:postgresql://" + host + ":" + port + "/" + databaseName);
        config.setDriverClassName("org.postgresql.Driver");
        config.setUsername(username);
        config.setPassword(password);
        config.setConnectionTimeout(20000);
        config.setIdleTimeout(300000);
        config.setMaxLifetime(1200000);
        config.setMaximumPoolSize(20);
        config.setMinimumIdle(10);
        config.setPoolName("ProdHikariPool");
        config.setLeakDetectionThreshold(60000);

        return new HikariDataSource(config);
    }

    /**
     * JdbcTemplate bean - available for all profiles
     * Uses SpEL to log the active environment
     */
    @Bean
    @ConditionalOnExpression("'${database.environment}' == 'development' or '${database.environment}' == 'production'")
    public JdbcTemplate jdbcTemplate(DataSource dataSource) {
        logger.info("Creating JdbcTemplate for environment: " + databaseEnvironment);
        return new JdbcTemplate(dataSource);
    }

    /**
     * Transaction Manager - available for all profiles
     */
    @Bean
    public PlatformTransactionManager transactionManager(DataSource dataSource) {
        logger.info("Creating DataSourceTransactionManager for environment: " + databaseEnvironment);
        return new DataSourceTransactionManager(dataSource);
    }

    /**
     * Database Info Bean - uses SpEL for conditional creation based on properties
     */
    @Bean
    public DatabaseInfo databaseInfo() {
        logger.info("Creating DatabaseInfo bean for environment: " + databaseEnvironment);
        return new DatabaseInfo(databaseEnvironment, host, port, databaseName, username);
    }

    /**
     * Inner class to hold database information
     */
    public static class DatabaseInfo {
        private final String environment;
        private final String host;
        private final int port;
        private final String databaseName;
        private final String username;

        public DatabaseInfo(String environment, String host, int port, String databaseName, String username) {
            this.environment = environment;
            this.host = host;
            this.port = port;
            this.databaseName = databaseName;
            this.username = username;
        }

        public String getEnvironment() { return environment; }
        public String getHost() { return host; }
        public int getPort() { return port; }
        public String getDatabaseName() { return databaseName; }
        public String getUsername() { return username; }

        @Override
        public String toString() {
            return "DatabaseInfo{" +
                    "environment='" + environment + '\'' +
                    ", host='" + host + '\'' +
                    ", port=" + port +
                    ", databaseName='" + databaseName + '\'' +
                    ", username='" + username + '\'' +
                    '}';
        }
    }
}