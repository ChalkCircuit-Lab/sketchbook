package ro.uaic.dbxdrgsl.OrderManagementSystem.event;

import org.springframework.context.event.EventListener;
import org.springframework.stereotype.Component;

@Component
public class BigDiscountListener {
    @EventListener
    public void onBigDiscount(BigDiscountEvent event) {
        System.out.printf("[EVENT] Large discount of %.2f for %s%n",
                event.getDiscount(), event.getCustomerName());
    }
}
