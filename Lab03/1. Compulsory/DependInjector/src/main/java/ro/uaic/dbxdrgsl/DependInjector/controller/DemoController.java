package ro.uaic.dbxdrgsl.DependInjector.controller;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import ro.uaic.dbxdrgsl.DependInjector.service.MessageService;
import ro.uaic.dbxdrgsl.DependInjector.service.TimeService;

@RestController
public class DemoController {

    private final MessageService messageService; // constructor injection

    @Autowired
    private TimeService timeService; // field injection

    private String extraInfo;

    // --- 1. Constructor injection ---
    @Autowired
    public DemoController(MessageService messageService) {
        System.out.println("1. Constructor Injection");
        this.messageService = messageService;
    }

    // --- 2. Setter injection ---
    @Autowired
    public void setExtraInfo() {
        System.out.println("3. Setter/Method Injection");
        this.extraInfo = "Setter injection executed";
    }

    @GetMapping("/demo")
    public String showInjectionOrder() {
        return """
            <html>
              <body>
                <h2>Dependency Injection Order Demo</h2>
                <p>1. %s</p>
                <p>2. %s</p>
                <p>3. %s</p>
              </body>
            </html>
            """.formatted(
                messageService.getMessage(),
                timeService.getCurrentTime(),
                extraInfo
        );
    }

    // --- 3. Field Injection triggers after constructor but before setter ---
    @Autowired
    public void init() {
        System.out.println("2. Field Injection");
    }
}
