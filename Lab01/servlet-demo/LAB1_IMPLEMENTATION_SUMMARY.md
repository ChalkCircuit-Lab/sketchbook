# Java Technologies Lab 1 - Implementation Summary

## ✅ All Requirements Fulfilled

### Compulsory Requi4. Check GlassFish server logs for request informationements (1p) ✓

1. **Java/Jakarta EE Server Installation**: Eclipse GlassFish 7.0.25 installed at `C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7` with Java 25
2. **Java EE Application**: `servlet-demo.war` file created and deployable
3. **Welcome Page Access**: Available at `http://localhost:8080/servlet-demo` after deployment

### Homework Requirements (2p) ✓

#### 1. Two Simple HTML Pages ✓
- **File**: `src/main/webapp/page1.html` - Simple HTML page with "Welcome to Page 1"
- **File**: `src/main/webapp/page2.html` - Simple HTML page with "Welcome to Page 2"

#### 2. Dynamic Welcome Page with Form ✓
- **File**: `src/main/webapp/index.jsp`
- **Features**: 
  - Form with radio buttons for selecting value 1 or 2
  - Submits to controller servlet via POST method
  - Uses proper `choice` parameter with values "1" or "2"

#### 3. Controller Servlet ✓
- **File**: `src/main/java/ro/uaic/dbxdrgsl/servletdemo/RouteServlet.java`
- **URL Pattern**: `/route`
- **Functionality**:
  - Receives form submissions via POST
  - Forwards to `page1.html` when choice=1
  - Forwards to `page2.html` when choice=2
  - Falls back to `index.jsp` for invalid choices

#### 4. Server Logging ✓
- **Implementation**: Enhanced logging in `RouteServlet.doPost()`
- **Logged Information**:
  - HTTP method (POST)
  - Client IP address
  - User-Agent header
  - Client language preferences
  - Request parameter (choice value)
  - Destination page

#### 5. Desktop Application Invocation ✓

##### Java Desktop Client ✓
- **File**: `desktop-clients/java/ServletClient.java`
- **Features**:
  - Uses Java 11+ HttpClient
  - Interactive command-line interface
  - Sends POST requests to `/api/choose`
  - Custom User-Agent identification
  - Error handling and timeout configuration

##### Python Desktop Client ✓
- **File**: `desktop-clients/python/servlet_client.py`
- **Features**:
  - Uses requests library
  - Interactive and automated demo modes
  - Proper error handling
  - Custom User-Agent identification

#### 6. Plain Text API Response ✓
- **File**: `src/main/java/ro/uaic/dbxdrgsl/servletdemo/ApiChooseServlet.java`
- **URL Pattern**: `/api/choose`
- **Functionality**:
  - Accepts both GET and POST requests
  - Returns plain text response (not HTML)
  - Returns the parameter value ("1" or "2")
  - Supports both `choice` parameter and button names

## File Structure

```
servlet-demo/
├── pom.xml                                    # Maven configuration
├── verify-requirements.bat                   # Verification script
├── src/main/
│   ├── java/ro/uaic/dbxdrgsl/servletdemo/
│   │   ├── RouteServlet.java                 # Main controller servlet
│   │   └── ApiChooseServlet.java             # API servlet for desktop clients
│   └── webapp/
│       ├── index.jsp                         # Dynamic welcome page with form
│       ├── page1.html                        # Target page 1
│       ├── page2.html                        # Target page 2
│       └── WEB-INF/web.xml                   # Web application descriptor
├── target/
│   └── servlet-demo.war                      # Deployable WAR file
└── desktop-clients/
    ├── README.md                             # Client documentation
    ├── java/
    │   └── ServletClient.java                # Java desktop client
    └── python/
        ├── servlet_client.py                 # Python desktop client
        └── requirements.txt                  # Python dependencies
```

## Deployment and Testing Instructions

### 1. Deploy to GlassFish
1. Start GlassFish server: `C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7\bin\asadmin start-domain`
2. Deploy the WAR file: `C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7\bin\asadmin deploy target\servlet-demo.war`
3. Access `http://localhost:8080/servlet-demo`
4. Alternative: Use GlassFish Admin Console at `http://localhost:4848`

### 2. Test Web Interface
1. Open browser to `http://localhost:8080/servlet-demo`
2. Select radio button for Page 1 or Page 2
3. Click "Navigate" button
4. Verify correct page is displayed
5. Check Tomcat logs for request information

### 3. Test Desktop Clients

#### Java Client
```bash
cd desktop-clients/java
javac ServletClient.java
java ServletClient
```

#### Python Client
```bash
cd desktop-clients/python
pip install -r requirements.txt
python servlet_client.py
# OR for automated demo:
python servlet_client.py --demo
```

### 4. API Testing
Direct API calls can be made to:
- `http://localhost:8080/servlet-demo/api/choose?choice=1`
- `http://localhost:8080/servlet-demo/api/choose?choice=2`

Expected response: Plain text "1" or "2"

## Technical Implementation Details

### Servlet Configuration
- Uses Jakarta EE 6.0 servlet API
- Annotation-based configuration (`@WebServlet`)
- No additional web.xml servlet mappings needed

### Request Handling
- Form submissions use POST method
- API endpoint accepts both GET and POST
- Proper parameter validation and fallback handling

### Logging Implementation
- Uses `java.util.logging.Logger`
- Logs all required information in structured format
- Includes request tracing for debugging

### Desktop Client Features
- Both clients demonstrate different HTTP client approaches
- Proper error handling for network issues
- Custom User-Agent strings for identification
- Interactive user interfaces

## Verification

Run the verification script to check all requirements:
```bash
verify-requirements.bat
```

This script will:
- Build the application with Maven
- Verify all required files exist
- Compile desktop clients
- Install Python dependencies
- Provide deployment instructions

## Status: ✅ ALL REQUIREMENTS COMPLETED AND VERIFIED

All compulsory and homework requirements have been successfully implemented, deployed, and tested.

### ✅ Verification Results:
- **GlassFish Server**: Running Eclipse GlassFish 7.0.25 with Java 25
- **Application Deployment**: ✅ servlet-demo deployed and accessible
- **Web Interface**: ✅ http://localhost:8080/servlet-demo working correctly
- **API Endpoint**: ✅ Returns plain text "1" or "2" as expected
- **Java Desktop Client**: ✅ Successfully communicates with servlet
- **Python Desktop Client**: ✅ Successfully communicates with servlet
- **Server Logging**: ✅ All request information logged properly
- **Form Navigation**: ✅ Routes correctly to page1.html and page2.html

### 🎯 Lab Requirements Status:
- **Compulsory (1p)**: ✅ COMPLETED - Server installed, application deployed, welcome page accessible
- **Homework (2p)**: ✅ COMPLETED - All 6 requirements fulfilled and working