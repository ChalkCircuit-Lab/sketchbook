package ro.uaic.dbxdrgsl.DroolsDiscounts.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@NoArgsConstructor
@AllArgsConstructor
public class Order {
    private double amount;
    private double discount;
    private Customer customer;
}
