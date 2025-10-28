package ro.uaic.dbxdrgsl.OrderManagementSystem.controller;

import org.springframework.web.bind.annotation.*;
import ro.uaic.dbxdrgsl.OrderManagementSystem.model.Order;
import ro.uaic.dbxdrgsl.OrderManagementSystem.service.OrderService;

@RestController
@RequestMapping("/orders")
public class OrderController {

    private final OrderService orderService;

    public OrderController(OrderService orderService) {
        this.orderService = orderService;
    }

    @GetMapping("/{id}")
    public String testOrder(@PathVariable Long id) {
        Order order = new Order(id, id, 600); // just for demo
        double discount = orderService.processOrder(order);
        return "Discount applied: " + discount;
    }
}

