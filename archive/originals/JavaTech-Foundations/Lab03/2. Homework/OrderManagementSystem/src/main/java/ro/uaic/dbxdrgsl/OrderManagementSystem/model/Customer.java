package ro.uaic.dbxdrgsl.OrderManagementSystem.model;

public class Customer {
    private Long id;
    private String name;
    private boolean loyal;

    public Customer(Long id, String name, boolean loyal) {
        this.id = id;
        this.name = name;
        this.loyal = loyal;
    }

    public Long getId() { return id; }
    public String getName() { return name; }
    public boolean isLoyal() { return loyal; }
}
