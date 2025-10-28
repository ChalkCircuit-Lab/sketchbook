package ro.uaic.dbxdrgsl.DroolsDiscounts.controller;

import org.springframework.web.bind.annotation.*;
import ro.uaic.dbxdrgsl.DroolsDiscounts.model.*;
import ro.uaic.dbxdrgsl.DroolsDiscounts.service.DiscountService;

@RestController
@RequestMapping("/orders")
public class OrderController {
    private final DiscountService discountService;

    public OrderController(DiscountService discountService) {
        this.discountService = discountService;
    }

    @GetMapping("/test")
    public String test(@RequestParam String name,
                       @RequestParam boolean loyal,
                       @RequestParam double total) {
        Customer c = new Customer(name, loyal);
        Order o = new Order(total);
        double d = discountService.applyDiscount(c, o);
        return String.format("Customer %s gets discount %.2f", name, d);
    }
}
