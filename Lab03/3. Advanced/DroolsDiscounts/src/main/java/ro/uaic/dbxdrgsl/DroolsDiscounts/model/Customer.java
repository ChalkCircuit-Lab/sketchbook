package ro.uaic.dbxdrgsl.DroolsDiscounts.model;

public class Customer {
    private String name;
    private boolean loyal;

    public Customer() {}
    public Customer(String name, boolean loyal) {
        this.name = name;
        this.loyal = loyal;
    }

    public String getName() { return name; }
    public boolean isLoyal() { return loyal; }
}
