package ro.uaic.dbxdrgsl.OrderManagementSystem.service;

import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;

@Service
@Profile("largeorder")
public class LargeOrderDiscountService implements DiscountService {
    @Override
    public double applyDiscount(Customer customer, Order order) {
        double discount = order.getTotalAmount() > 500 ? 50 : 0;
        System.out.println("LargeOrderDiscountService applied: " + discount);
        return discount;
    }
}
