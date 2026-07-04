package ro.uaic.dbxdrgsl.springboot_demo.config;
import org.springframework.boot.context.properties.ConfigurationProperties;

@ConfigurationProperties(prefix = "app.db")
public class AppDbProperties {
    private String vendor; // h2 or mysql
    private String host;
    private int port;
    private String name;
    private String user;
    private String password;

    // getters/setters
    public String getVendor(){return vendor;} public void setVendor(String v){this.vendor=v;}
    public String getHost(){return host;} public void setHost(String v){this.host=v;}
    public int getPort(){return port;} public void setPort(int v){this.port=v;}
    public String getName(){return name;} public void setName(String v){this.name=v;}
    public String getUser(){return user;} public void setUser(String v){this.user=v;}
    public String getPassword(){return password;} public void setPassword(String v){this.password=v;}
}
