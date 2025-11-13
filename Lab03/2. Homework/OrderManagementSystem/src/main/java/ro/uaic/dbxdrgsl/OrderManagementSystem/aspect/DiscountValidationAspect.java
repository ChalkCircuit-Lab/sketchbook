package ro.uaic.dbxdrgsl.OrderManagementSystem.aspect;

import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.annotation.*;
import org.springframework.stereotype.Component;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;
import ro.uaic.dbxdrgsl.OrderManagementSystem.repository.CustomerRepository;
import ro.uaic.dbxdrgsl.OrderManagementSystem.exception.CustomerNotFoundException;
import ro.uaic.dbxdrgsl.OrderManagementSystem.exception.CustomerNotEligibleException;

@Aspect
@Component
public class DiscountValidationAspect {

    private final CustomerRepository repository;

    public DiscountValidationAspect(CustomerRepository repository) {
        this.repository = repository;
    }

    @Before("execution(* ro.uaic.dbxdrgsl.OrderManagementSystem.service.DiscountService.applyDiscount(..))")
    public void checkEligibility(JoinPoint joinPoint) {
        Object[] args = joinPoint.getArgs();
        Customer c = (Customer) args[0];
        Order o = (Order) args[1];

        // check existence
        if (!repository.findById(c.getId()).isPresent()) {
            throw new CustomerNotFoundException("Customer with id " + c.getId() + " not found");
        }

        // simple eligibility rule: either loyal or order >= 100
        if (!c.isLoyal() && o.getTotalAmount() < 100) {
            throw new CustomerNotEligibleException("Customer " + c.getName() + " not eligible for discount");
        }

        System.out.println("[ASPECT] Eligibility OK for " + c.getName());
    }
}
