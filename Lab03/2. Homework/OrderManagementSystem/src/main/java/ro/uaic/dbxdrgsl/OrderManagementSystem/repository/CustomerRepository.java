package ro.uaic.dbxdrgsl.OrderManagementSystem.repository;

import org.springframework.stereotype.Repository;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Customer;
import java.util.*;

@Repository
public class CustomerRepository {
    private final Map<Long, Customer> customers = new HashMap<>();

    public CustomerRepository() {
        customers.put(1L, new Customer(1L, "Alice", true));
        customers.put(2L, new Customer(2L, "Bob", false));
    }

    public Optional<Customer> findById(Long id) {
        return Optional.ofNullable(customers.get(id));
    }
}
