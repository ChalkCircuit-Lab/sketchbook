package ro.uaic.dbxdrgsl.controller;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.*;
import org.springframework.cloud.context.config.annotation.RefreshScope;

@RefreshScope
@RestController
public class MessageController {

    @Value("${message}")
    private String message;

    @GetMapping("/message")
    public String getMessage() {
        return "Service A -> " + message;
    }
}
