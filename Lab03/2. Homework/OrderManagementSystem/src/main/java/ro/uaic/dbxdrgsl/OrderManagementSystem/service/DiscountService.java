package ro.uaic.dbxdrgsl.OrderManagementSystem.service;

import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;

public interface DiscountService {
    double applyDiscount(Customer customer, Order order);
}
