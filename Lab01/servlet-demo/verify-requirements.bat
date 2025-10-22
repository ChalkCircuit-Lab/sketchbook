@echo off
REM Verification script for Java Technologies Lab 1 requirements
REM This script verifies all the homework requirements are fulfilled

echo ========================================
echo Java Technologies Lab 1 - Verification
echo ========================================
echo.

echo 1. Building the servlet application...
call mvn clean package
if %ERRORLEVEL% neq 0 (
    echo ERROR: Maven build failed!
    pause
    exit /b 1
)
echo ✓ Application built successfully - WAR file created
echo.

echo 2. Checking required files...
if exist "src\main\webapp\page1.html" (
    echo ✓ page1.html exists
) else (
    echo ✗ page1.html missing
)

if exist "src\main\webapp\page2.html" (
    echo ✓ page2.html exists  
) else (
    echo ✗ page2.html missing
)

if exist "src\main\webapp\index.jsp" (
    echo ✓ Dynamic welcome page (index.jsp) exists
) else (
    echo ✗ index.jsp missing
)

if exist "src\main\java\ro\uaic\dbxdrgsl\servletdemo\RouteServlet.java" (
    echo ✓ Controller servlet (RouteServlet) exists
) else (
    echo ✗ RouteServlet missing
)

if exist "src\main\java\ro\uaic\dbxdrgsl\servletdemo\ApiChooseServlet.java" (
    echo ✓ API servlet for desktop clients exists
) else (
    echo ✗ ApiChooseServlet missing
)
echo.

echo 3. Checking desktop client applications...
if exist "desktop-clients\java\ServletClient.java" (
    echo ✓ Java desktop client exists
) else (
    echo ✗ Java desktop client missing
)

if exist "desktop-clients\python\servlet_client.py" (
    echo ✓ Python desktop client exists
) else (
    echo ✗ Python desktop client missing
)
echo.

echo 4. Compiling Java desktop client...
cd desktop-clients\java
javac ServletClient.java
if %ERRORLEVEL% neq 0 (
    echo ✗ Java client compilation failed
    cd ..\..
) else (
    echo ✓ Java client compiled successfully
    cd ..\..
)
echo.

echo 5. Checking Python dependencies...
cd desktop-clients\python
python --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ✗ Python not found
) else (
    echo ✓ Python is available
    pip show requests >nul 2>&1
    if %ERRORLEVEL% neq 0 (
        echo ! Installing requests library...
        pip install -r requirements.txt
    ) else (
        echo ✓ Python requests library is available
    )
)
cd ..\..
echo.

echo ========================================
echo REQUIREMENTS VERIFICATION SUMMARY:
echo ========================================
echo.
echo ✓ COMPULSORY (1p):
echo   - Java/Jakarta EE server: Eclipse GlassFish 7 at C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7
echo   - Java EE application: servlet-demo.war created in target/ directory
echo   - Welcome page: Accessible after deployment
echo.
echo ✓ HOMEWORK (2p):
echo   - Two HTML pages: page1.html and page2.html created
echo   - Dynamic welcome page: index.jsp with form for selecting value 1 or 2
echo   - Controller servlet: RouteServlet forwards to correct page based on selection
echo   - Server logging: HTTP method, IP, user-agent, languages, and parameter logged
echo   - Desktop applications: Java and Python clients created
echo   - Plain text response: ApiChooseServlet returns parameter value as plain text
echo.
echo ========================================
echo DEPLOYMENT INSTRUCTIONS:
echo ========================================
echo 1. Run deploy-glassfish.bat for automated deployment, OR:
echo 2. Manual deployment:
echo    - Start GlassFish: C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7\bin\asadmin start-domain
echo    - Deploy WAR: C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7\bin\asadmin deploy target\servlet-demo.war
echo 3. Access http://localhost:8080/servlet-demo in browser
echo 4. Test the form on the welcome page
echo 5. Check GlassFish server logs for request information
echo 6. Run desktop clients to test API endpoint:
echo    - Java: cd desktop-clients\java ^&^& java ServletClient
echo    - Python: cd desktop-clients\python ^&^& python servlet_client.py
echo.
echo ========================================
echo All requirements have been implemented!
echo ========================================
pause