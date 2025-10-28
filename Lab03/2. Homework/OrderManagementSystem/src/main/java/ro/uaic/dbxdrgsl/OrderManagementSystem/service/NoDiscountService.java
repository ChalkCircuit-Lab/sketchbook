package ro.uaic.dbxdrgsl.OrderManagementSystem.service;

import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;

@Service
@Profile("default")
public class NoDiscountService implements DiscountService {
    @Override
    public double applyDiscount(Customer customer, Order order) {
        System.out.println("No discount applied.");
        return 0;
    }
}
