package ro.uaic.dbxdrgsl.springboot_demo.actuator;

import org.springframework.boot.actuate.info.Info;
import org.springframework.boot.actuate.info.InfoContributor;
import org.springframework.core.env.Environment;
import org.springframework.stereotype.Component;

@Component
public class DatabaseInfoContributor implements InfoContributor {
    private final Environment env;
    public DatabaseInfoContributor(Environment env){ this.env = env; }

    @Override
    public void contribute(Info.Builder builder) {
        builder.withDetail("database",
                java.util.Map.of(
                        "vendor", env.getProperty("app.db.vendor","unknown"),
                        "host", env.getProperty("app.db.host",""),
                        "name", env.getProperty("app.db.name",""),
                        "profile", String.join(",", env.getActiveProfiles())
                )
        );
    }
}
