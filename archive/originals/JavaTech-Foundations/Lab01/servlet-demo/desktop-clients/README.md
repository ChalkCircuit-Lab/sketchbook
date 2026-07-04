# Desktop Clients for Servlet Demo

This directory contains desktop client applications that demonstrate invoking the servlet from different programming languages.

## Java Client

**Location**: `java/ServletClient.java`

**Requirements**: Java 11 or higher (uses `java.net.http.HttpClient`)

**How to run**:
1. Compile: `javac ServletClient.java`
2. Run: `java ServletClient`

**Features**:
- Uses Java 11+ HttpClient for HTTP requests
- Interactive command-line interface
- Sends POST requests to `/api/choose` endpoint
- Displays HTTP status and headers
- Returns plain text response from servlet

## Python Client

**Location**: `python/servlet_client.py`

**Requirements**: 
- Python 3.6+
- requests library

**How to setup and run**:
1. Install dependencies: `pip install -r requirements.txt`
2. Run interactive mode: `python servlet_client.py`
3. Run automated demo: `python servlet_client.py --demo`

**Features**:
- Uses requests library for HTTP communication
- Interactive command-line interface
- Automated demo mode for testing
- Custom User-Agent identification
- Error handling and timeout configuration

## Testing the Clients

1. **Start the servlet application**:
   - Make sure GlassFish is running (use `setup-and-deploy.bat` for automatic deployment)
   - Deploy the `servlet-demo.war` file
   - Verify the application is accessible at `http://localhost:8080/servlet-demo`

2. **Test the API endpoint**:
   - Browser: `http://localhost:8080/servlet-demo/api/choose?choice=1`
   - Should return plain text "1"

3. **Run the desktop clients**:
   - Both clients will prompt for input (1 or 2)
   - The servlet should return the same value as plain text
   - Check server logs for request information

## Expected Behavior

When a desktop client sends a request:
- Client sends POST request with `choice=1` or `choice=2`
- Servlet responds with plain text "1" or "2"
- Server logs the request details (method, IP, user-agent, etc.)
- No HTML content is returned (unlike web browser requests)