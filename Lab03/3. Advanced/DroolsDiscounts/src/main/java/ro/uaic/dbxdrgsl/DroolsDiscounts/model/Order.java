package ro.uaic.dbxdrgsl.DroolsDiscounts.model;

public class Order {
    private double total;
    private double discount;

    public Order(double total) {
        this.total = total;
    }

    public double getTotal() { return total; }
    public double getDiscount() { return discount; }
    public void setDiscount(double discount) { this.discount = discount; }
}
