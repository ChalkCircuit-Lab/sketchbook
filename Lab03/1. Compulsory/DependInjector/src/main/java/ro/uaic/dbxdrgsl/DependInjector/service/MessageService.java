package ro.uaic.dbxdrgsl.DependInjector.service;

import org.springframework.stereotype.Service;

@Service
public class MessageService {
    public String getMessage() {
        return "Injected message from MessageService";
    }
}
