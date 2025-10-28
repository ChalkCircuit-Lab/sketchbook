package ro.uaic.dbxdrgsl.OrderManagementSystem.service;

import org.springframework.context.ApplicationEventPublisher;
import org.springframework.stereotype.Service;
import ro.uaic.dbxdrgsl.OrderManagementSystem.event.BigDiscountEvent;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;
import ro.uaic.dbxdrgsl.OrderManagementSystem.repository.CustomerRepository;

@Service
public class OrderService {

    private final DiscountService discountService;
    private final CustomerRepository repository;
    private final ApplicationEventPublisher publisher;

    public OrderService(DiscountService discountService, CustomerRepository repository, ApplicationEventPublisher publisher) {
        this.discountService = discountService;
        this.repository = repository;
        this.publisher = publisher;
    }

    public double processOrder(Order order) {
        Customer customer = repository.findById(order.getCustomerId())
                .orElseThrow(() -> new RuntimeException("Customer not found"));

        double discount = discountService.applyDiscount(customer, order);

        System.out.printf("Discount applied by %s for %s: %.2f%n",
                discountService.getClass().getSimpleName(),
                customer.getName(),
                discount);

        if (discount > 40)
            publisher.publishEvent(new BigDiscountEvent(this, discount, customer.getName()));

        return discount;
    }
}
