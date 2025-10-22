-- Test data for development environment
INSERT INTO users (username, email, full_name, created_at) VALUES
    ('admin', 'admin@example.com', 'System Administrator', CURRENT_TIMESTAMP),
    ('john.doe', 'john.doe@example.com', 'John Doe', CURRENT_TIMESTAMP),
    ('jane.smith', 'jane.smith@example.com', 'Jane Smith', CURRENT_TIMESTAMP),
    ('bob.wilson', 'bob.wilson@example.com', 'Bob Wilson', CURRENT_TIMESTAMP),
    ('alice.johnson', 'alice.johnson@example.com', 'Alice Johnson', CURRENT_TIMESTAMP);