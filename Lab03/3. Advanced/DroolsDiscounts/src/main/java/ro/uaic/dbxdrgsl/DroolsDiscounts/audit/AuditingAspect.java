package ro.uaic.dbxdrgsl.DroolsDiscounts.audit;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.*;
import org.springframework.stereotype.Component;

@Aspect
@Component
public class AuditingAspect {

    @AfterReturning(pointcut = "execution(* ro.uaic.dbxdrgsl.DroolsDiscounts.service.DiscountService.applyDiscount(..))",
            returning = "discount")
    public void logAudit(JoinPoint jp, Object discount) {
        System.out.printf("[AUDIT] %s executed. Result: %.2f%n",
                jp.getSignature().getName(),
                discount);
    }
}
