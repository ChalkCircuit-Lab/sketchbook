package ro.uaic.dbxdrgsl.OrderManagementSystem.model;

public class Order {
    private Long id;
    private Long customerId;
    private double totalAmount;

    public Order(Long id, Long customerId, double totalAmount) {
        this.id = id;
        this.customerId = customerId;
        this.totalAmount = totalAmount;
    }

    public Long getId() { return id; }
    public Long getCustomerId() { return customerId; }
    public double getTotalAmount() { return totalAmount; }
}
