package uaic.dbxdrgsl.PrefSchedule.service;

import com.stablematch.dto.StableMatchingRequestDTO;
import com.stablematch.dto.StableMatchingResponseDTO;
import io.github.resilience4j.retry.Retry;
import io.github.resilience4j.retry.annotation.Retry;
import io.github.resilience4j.timelimiter.annotation.TimeLimiter;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.MediaType;
import org.springframework.stereotype.Service;
import org.springframework.web.reactive.function.client.WebClient;
import org.springframework.web.reactive.function.client.WebClientResponseException;
import reactor.core.publisher.Mono;
import reactor.util.retry.Retry as ReactorRetry;

import java.time.Duration;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeoutException;

/**
 * Service for communicating with the StableMatch microservice.
 * Implements resilience patterns: Retry, Timeout, and Fallback.
 */
@Slf4j
@Service
@RequiredArgsConstructor
public class StableMatchClient {

    private final WebClient webClient;
    private final RandomMatchingService randomMatchingService;

    @Value("${stable-match.url:http://localhost:8080/api}")
    private String stableMatchUrl;

    @Value("${stable-match.timeout:5000}")
    private long timeoutMs;

    @Value("${stable-match.retry.max-attempts:3}")
    private int maxRetryAttempts;

    @Value("${stable-match.retry.delay:500}")
    private long retryDelayMs;

    /**
     * Invoke the StableMatch service with resilience patterns.
     * Implements: Retry, Timeout, and Fallback (random matching).
     *
     * @param request the matching request
     * @return the matching response
     */
    @Retry(name = "stableMatchRetry", fallbackMethod = "fallbackMatching")
    @TimeLimiter(name = "stableMatchTimeout", fallbackMethod = "fallbackMatching")
    public CompletableFuture<StableMatchingResponseDTO> invokeStableMatching(
            StableMatchingRequestDTO request) {

        log.info("Invoking StableMatch service with {} students and {} courses",
                request.getStudents().size(), request.getCourses().size());

        return webClient.post()
                .uri(stableMatchUrl + "/v1/matching/solve")
                .contentType(MediaType.APPLICATION_JSON)
                .bodyValue(request)
                .retrieve()
                .bodyToMono(StableMatchingResponseDTO.class)
                .timeout(Duration.ofMillis(timeoutMs))
                .retryWhen(ReactorRetry.backoff(maxRetryAttempts - 1, Duration.ofMillis(retryDelayMs))
                        .maxBackoff(Duration.ofSeconds(2))
                        .doBeforeRetry(signal -> log.warn("Retrying StableMatch invocation, attempt: {}",
                                signal.totalRetries() + 1))
                        .onRetryExhaustedThrow((spec, signal) -> {
                            log.error("StableMatch invocation failed after {} attempts", maxRetryAttempts);
                            return new RuntimeException("Max retry attempts exceeded for StableMatch service");
                        }))
                .doOnError(error -> log.error("Error invoking StableMatch service: {}", error.getMessage()))
                .toFuture();
    }

    /**
     * Synchronous wrapper for invoking StableMatch service
     * Uses the async method internally
     *
     * @param request the matching request
     * @return the matching response
     */
    public StableMatchingResponseDTO solveMatching(StableMatchingRequestDTO request) {
        try {
            return invokeStableMatching(request).get();
        } catch (TimeoutException e) {
            log.error("Timeout waiting for StableMatch response: {}", e.getMessage());
            return fallbackMatching(request, new TimeoutException("StableMatch timeout"));
        } catch (Exception e) {
            log.error("Error invoking StableMatch: {}", e.getMessage());
            return fallbackMatching(request, e);
        }
    }

    /**
     * Fallback method when StableMatch service fails.
     * Uses random matching as a backup strategy.
     *
     * @param request the original matching request
     * @param throwable the exception that occurred
     * @return random matching result
     */
    public CompletableFuture<StableMatchingResponseDTO> fallbackMatching(
            StableMatchingRequestDTO request,
            Throwable throwable) {

        log.warn("Falling back to random matching due to: {}", throwable.getMessage());
        log.info("StableMatch service unavailable or timeout occurred, using random fallback algorithm");

        StableMatchingResponseDTO randomResult = randomMatchingService.generateRandomMatching(request);
        // Mark response as fallback
        randomResult.setMessage(randomResult.getMessage() + " (StableMatch unavailable, used random fallback)");

        return CompletableFuture.completedFuture(randomResult);
    }

    /**
     * Overloaded fallback for synchronous context
     */
    public StableMatchingResponseDTO fallbackMatching(
            StableMatchingRequestDTO request,
            Exception exception) {

        log.warn("Synchronous fallback to random matching due to: {}", exception.getMessage());
        return randomMatchingService.generateRandomMatching(request);
    }
}
