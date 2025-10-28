package ro.uaic.dbxdrgsl.OrderManagementSystem.event;

import org.springframework.context.ApplicationEvent;

public class BigDiscountEvent extends ApplicationEvent {
    private final double discount;
    private final String customerName;

    public BigDiscountEvent(Object source, double discount, String customerName) {
        super(source);
        this.discount = discount;
        this.customerName = customerName;
    }

    public double getDiscount() { return discount; }
    public String getCustomerName() { return customerName; }
}
