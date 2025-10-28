package ro.uaic.dbxdrgsl.DependInjector.service;

import org.springframework.stereotype.Service;
import java.time.LocalTime;

@Service
public class TimeService {
    public String getCurrentTime() {
        return "Current time: " + LocalTime.now();
    }
}
