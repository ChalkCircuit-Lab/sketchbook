package ro.uaic.dbxdrgsl.DroolsDiscounts.controller;

import org.springframework.web.bind.annotation.*;
import ro.uaic.dbxdrgsl.DroolsDiscounts.model.Order;
import ro.uaic.dbxdrgsl.DroolsDiscounts.service.DiscountRuleService;

@RestController
@RequestMapping("/orders")
public class OrderController {

    private final DiscountRuleService discountRuleService;

    public OrderController(DiscountRuleService discountRuleService) {
        this.discountRuleService = discountRuleService;
    }

    @PostMapping("/apply")
    public Order applyDiscount(@RequestBody Order order) {
        return discountRuleService.applyDiscount(order);
    }
}
