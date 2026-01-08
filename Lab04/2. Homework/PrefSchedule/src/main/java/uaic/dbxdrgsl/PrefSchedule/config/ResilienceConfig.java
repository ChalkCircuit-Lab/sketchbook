package uaic.dbxdrgsl.PrefSchedule.config;

import io.github.resilience4j.core.registry.EntryAddedEvent;
import io.github.resilience4j.core.registry.EntryRemovedEvent;
import io.github.resilience4j.core.registry.EntryReplacedEvent;
import io.github.resilience4j.core.registry.RegistryEventConsumer;
import io.github.resilience4j.retry.Retry;
import io.github.resilience4j.retry.RetryRegistry;
import io.github.resilience4j.timelimiter.TimeLimiter;
import io.github.resilience4j.timelimiter.TimeLimiterRegistry;
import lombok.extern.slf4j.Slf4j;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * Configuration for Resilience4j patterns: Retry, Timeout, and Fallback
 */
@Slf4j
@Configuration
public class ResilienceConfig {

    /**
     * Configure retry registry with event logging
     */
    @Bean
    public RegistryEventConsumer<Retry> retryRegistryEventConsumer() {
        return new RegistryEventConsumer<Retry>() {
            @Override
            public void onEntryAdded(EntryAddedEvent<Retry> entryAddedEvent) {
                log.info("Retry added: {}", entryAddedEvent.getAddedEntry().getName());
            }

            @Override
            public void onEntryRemoved(EntryRemovedEvent<Retry> entryRemoveEvent) {
                log.info("Retry removed: {}", entryRemoveEvent.getRemovedEntry().getName());
            }

            @Override
            public void onEntryReplaced(EntryReplacedEvent<Retry> entryReplacedEvent) {
                log.info("Retry replaced: {}", entryReplacedEvent.getNewEntry().getName());
            }
        };
    }

    /**
     * Configure time limiter registry with event logging
     */
    @Bean
    public RegistryEventConsumer<TimeLimiter> timeLimiterRegistryEventConsumer() {
        return new RegistryEventConsumer<TimeLimiter>() {
            @Override
            public void onEntryAdded(EntryAddedEvent<TimeLimiter> entryAddedEvent) {
                log.info("TimeLimiter added: {}", entryAddedEvent.getAddedEntry().getName());
            }

            @Override
            public void onEntryRemoved(EntryRemovedEvent<TimeLimiter> entryRemoveEvent) {
                log.info("TimeLimiter removed: {}", entryRemoveEvent.getRemovedEntry().getName());
            }

            @Override
            public void onEntryReplaced(EntryReplacedEvent<TimeLimiter> entryReplacedEvent) {
                log.info("TimeLimiter replaced: {}", entryReplacedEvent.getNewEntry().getName());
            }
        };
    }
}
