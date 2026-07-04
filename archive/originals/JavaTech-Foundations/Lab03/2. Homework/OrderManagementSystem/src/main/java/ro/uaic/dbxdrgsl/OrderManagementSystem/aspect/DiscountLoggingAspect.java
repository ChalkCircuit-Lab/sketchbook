package ro.uaic.dbxdrgsl.OrderManagementSystem.aspect;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.AfterReturning;
import org.aspectj.lang.annotation.Aspect;
import org.springframework.stereotype.Component;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;

@Aspect
@Component
public class DiscountLoggingAspect {

    @AfterReturning(pointcut = "execution(* ro.uaic.dbxdrgsl.OrderManagementSystem.service.DiscountService.applyDiscount(..))", returning = "discount")
    public void logDiscount(JoinPoint joinPoint, Object discount) {
        Object[] args = joinPoint.getArgs();
        Customer c = (Customer) args[0];
        // Order o = (Order) args[1];

        String method = joinPoint.getSignature().getName();
        double d = discount instanceof Number ? ((Number) discount).doubleValue() : 0.0;

        System.out.printf("[LOG] method=%s, customer=%s, discount=%.2f%n", method, c.getName(), d);
    }
}
