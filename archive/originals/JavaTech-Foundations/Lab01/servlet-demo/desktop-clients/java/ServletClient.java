import java.io.IOException;
import java.net.URI;
import java.net.URLEncoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Scanner;

/**
 * Java desktop client that invokes the servlet API.
 * This demonstrates invoking the servlet from a desktop application.
 */
public class ServletClient {
    
    private static final String BASE_URL = "http://localhost:8080/servlet-demo";
    private static final String API_ENDPOINT = "/api/choose";
    
    private final HttpClient httpClient;
    
    public ServletClient() {
        this.httpClient = HttpClient.newBuilder()
                .connectTimeout(Duration.ofSeconds(10))
                .build();
    }
    
    public String sendChoice(String choice) throws IOException, InterruptedException {
        // Prepare POST data
        String formData = "choice=" + URLEncoder.encode(choice, StandardCharsets.UTF_8);
        
        HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create(BASE_URL + API_ENDPOINT))
                .header("Content-Type", "application/x-www-form-urlencoded")
                .header("User-Agent", "Java Desktop Client/1.0")
                .POST(HttpRequest.BodyPublishers.ofString(formData))
                .build();
        
        HttpResponse<String> response = httpClient.send(request, 
                HttpResponse.BodyHandlers.ofString());
        
        System.out.println("HTTP Status: " + response.statusCode());
        System.out.println("Response Headers: " + response.headers().map());
        
        return response.body();
    }
    
    public static void main(String[] args) {
        ServletClient client = new ServletClient();
        Scanner scanner = new Scanner(System.in);
        
        System.out.println("=== Java Servlet Desktop Client ===");
        System.out.println("This client will send HTTP requests to the servlet API.");
        System.out.println("Make sure the servlet application is running on " + BASE_URL);
        System.out.println();
        
        while (true) {
            System.out.print("Enter choice (1 or 2), or 'quit' to exit: ");
            String input = scanner.nextLine().trim();
            
            if ("quit".equalsIgnoreCase(input)) {
                break;
            }
            
            if (!"1".equals(input) && !"2".equals(input)) {
                System.out.println("Invalid choice. Please enter 1 or 2.");
                continue;
            }
            
            try {
                System.out.println("Sending choice '" + input + "' to servlet...");
                String response = client.sendChoice(input);
                System.out.println("Servlet response: '" + response + "'");
                System.out.println();
                
            } catch (IOException | InterruptedException e) {
                System.err.println("Error communicating with servlet: " + e.getMessage());
                System.out.println("Make sure the servlet application is running.");
                System.out.println();
            }
        }
        
        scanner.close();
        System.out.println("Client terminated.");
    }
}