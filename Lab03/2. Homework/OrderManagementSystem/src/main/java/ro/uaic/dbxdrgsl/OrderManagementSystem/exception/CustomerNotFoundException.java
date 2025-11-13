package ro.uaic.dbxdrgsl.OrderManagementSystem.exception;

public class CustomerNotFoundException extends RuntimeException {
    public CustomerNotFoundException(String message) {
        super(message);
    }
}
