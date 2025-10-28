package ro.uaic.dbxdrgsl.OrderManagementSystem.aspect;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.*;
import org.springframework.stereotype.Component;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;

@Aspect
@Component
public class DiscountValidationAspect {

    @Before("execution(* ro.uaic.dbxdrgsl.OrderManagementSystem.service.DiscountService.applyDiscount(..))")
    public void checkEligibility(JoinPoint joinPoint) {
        Object[] args = joinPoint.getArgs();
        Customer c = (Customer) args[0];
        Order o = (Order) args[1];

        if (!c.isLoyal() && o.getTotalAmount() < 100) {
            throw new RuntimeException("Customer " + c.getName() + " not eligible for discount");
        }

        System.out.println("[ASPECT] Eligibility OK for " + c.getName());
    }
}
