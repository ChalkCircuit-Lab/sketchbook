package ro.uaic.dbxdrgsl.DroolsDiscounts.service;

import ro.uaic.dbxdrgsl.DroolsDiscounts.model.Customer;
import ro.uaic.dbxdrgsl.DroolsDiscounts.model.Order;

public interface DiscountService {
    double applyDiscount(Customer customer, Order order);
}
