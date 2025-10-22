#!/usr/bin/env python3
"""
Python desktop client that invokes the servlet API.
This demonstrates invoking the servlet from a Python desktop application.

Requirements:
- Python 3.6+
- requests library (install with: pip install requests)
"""

import requests
import sys

class ServletClient:
    """Client for communicating with the Java servlet API."""
    
    def __init__(self, base_url="http://localhost:8080/servlet-demo"):
        self.base_url = base_url
        self.api_endpoint = "/api/choose"
        self.session = requests.Session()
        # Set a custom user agent to identify this as a Python client
        self.session.headers.update({
            'User-Agent': 'Python Desktop Client/1.0'
        })
    
    def send_choice(self, choice):
        """Send choice to servlet and return the response."""
        url = self.base_url + self.api_endpoint
        data = {'choice': choice}
        
        try:
            print(f"Sending choice '{choice}' to {url}")
            response = self.session.post(url, data=data, timeout=10)
            
            print(f"HTTP Status: {response.status_code}")
            print(f"Response Headers: {dict(response.headers)}")
            
            response.raise_for_status()  # Raise exception for bad status codes
            return response.text
            
        except requests.exceptions.RequestException as e:
            raise Exception(f"Error communicating with servlet: {e}")
    
    def close(self):
        """Close the session."""
        self.session.close()

def main():
    """Main function for interactive client."""
    print("=== Python Servlet Desktop Client ===")
    print("This client will send HTTP requests to the servlet API.")
    
    client = ServletClient()
    print(f"Make sure the servlet application is running on {client.base_url}")
    print()
    
    try:
        while True:
            user_input = input("Enter choice (1 or 2), or 'quit' to exit: ").strip()
            
            if user_input.lower() == 'quit':
                break
            
            if user_input not in ['1', '2']:
                print("Invalid choice. Please enter 1 or 2.")
                continue
            
            try:
                response = client.send_choice(user_input)
                print(f"Servlet response: '{response}'")
                print()
                
            except Exception as e:
                print(f"Error: {e}")
                print("Make sure the servlet application is running.")
                print()
    
    except KeyboardInterrupt:
        print("\nReceived interrupt signal.")
    
    finally:
        client.close()
        print("Client terminated.")

def demo_automated():
    """Automated demo function that tests both choices."""
    print("=== Automated Demo ===")
    client = ServletClient()
    
    try:
        for choice in ['1', '2']:
            print(f"\nTesting choice: {choice}")
            response = client.send_choice(choice)
            print(f"Response: '{response}'")
            
            # Verify response matches expected value
            if response.strip() == choice:
                print("✓ Response matches expected value")
            else:
                print(f"✗ Expected '{choice}', got '{response.strip()}'")
    
    except Exception as e:
        print(f"Demo failed: {e}")
    
    finally:
        client.close()

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--demo":
        demo_automated()
    else:
        main()