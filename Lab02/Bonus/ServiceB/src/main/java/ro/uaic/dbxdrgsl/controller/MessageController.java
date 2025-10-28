package ro.uaic.dbxdrgsl.controller;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.cloud.context.config.annotation.RefreshScope;
import org.springframework.web.bind.annotation.*;

@RefreshScope
@RestController
public class MessageController {

    @Value("${message}")
    private String message;

    @GetMapping("/message")
    public String getMessage() {
        return "Service B -> " + message;
    }
}
