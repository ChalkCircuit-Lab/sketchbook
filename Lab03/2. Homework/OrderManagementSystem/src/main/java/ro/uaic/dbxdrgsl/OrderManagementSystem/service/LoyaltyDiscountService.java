package ro.uaic.dbxdrgsl.OrderManagementSystem.service;

import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;

@Service
@Profile("loyalty")
public class LoyaltyDiscountService implements DiscountService {
    @Override
    public double applyDiscount(Customer customer, Order order) {
        double discount = order.getTotalAmount() * 0.1;
        System.out.println("LoyaltyDiscountService applied: " + discount);
        return discount;
    }
}
