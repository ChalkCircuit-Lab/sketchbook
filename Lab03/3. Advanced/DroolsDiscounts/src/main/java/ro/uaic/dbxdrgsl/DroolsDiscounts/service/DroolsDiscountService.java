package ro.uaic.dbxdrgsl.DroolsDiscounts.service;

import org.kie.api.runtime.KieSession;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.DroolsDiscounts.model.Customer;
import ro.uaic.dbxdrgsl.DroolsDiscounts.model.Order;

@Service
public class DroolsDiscountService implements DiscountService {

    private final KieSession kieSession;

    public DroolsDiscountService(KieSession kieSession) {
        this.kieSession = kieSession;
    }

    @Override
    public double applyDiscount(Customer customer, Order order) {
        kieSession.insert(customer);
        kieSession.insert(order);
        kieSession.fireAllRules();
        System.out.printf("[SERVICE] Discount for %s = %.2f%n",
                customer.getName(), order.getDiscount());
        return order.getDiscount();
    }
}
