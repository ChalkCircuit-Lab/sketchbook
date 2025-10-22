@echo off
REM GlassFish Deployment Script for servlet-demo
REM This script automates the deployment process for GlassFish 7

set JAVA_HOME=C:\Program Files\Java\jdk-25
set GLASSFISH_HOME=C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7
set ASADMIN=%GLASSFISH_HOME%\bin\asadmin.bat
set PATH=%JAVA_HOME%\bin;%PATH%

echo ========================================
echo GlassFish Deployment Script
echo ========================================
echo.

echo GlassFish installation: %GLASSFISH_HOME%
echo.

echo 1. Building the application...
call mvn clean package
if %ERRORLEVEL% neq 0 (
    echo ERROR: Maven build failed!
    pause
    exit /b 1
)
echo ✓ Application built successfully
echo.

echo 2. Checking GlassFish server status...
call "%ASADMIN%" list-domains
echo.

echo 3. Starting GlassFish domain (if not running)...
call "%ASADMIN%" start-domain
echo.

echo 4. Undeploying previous version (if exists)...
call "%ASLADMIN%" undeploy servlet-demo 2>nul
echo Previous deployment removed (if existed)
echo.

echo 5. Deploying servlet-demo.war...
call "%ASADMIN%" deploy target\servlet-demo.war
if %ERRORLEVEL% neq 0 (
    echo ERROR: Deployment failed!
    pause
    exit /b 1
)
echo ✓ Application deployed successfully
echo.

echo 6. Listing deployed applications...
call "%ASADMIN%" list-applications
echo.

echo ========================================
echo DEPLOYMENT COMPLETE!
echo ========================================
echo.
echo Application URL: http://localhost:8080/servlet-demo
echo Admin Console: http://localhost:4848
echo.
echo To test the application:
echo 1. Open browser to http://localhost:8080/servlet-demo
echo 2. Test the form with radio buttons
echo 3. Check server logs for request information
echo 4. Test API endpoint: http://localhost:8080/servlet-demo/api/choose?choice=1
echo.
echo To test desktop clients:
echo - Java: cd desktop-clients\java ^&^& java ServletClient
echo - Python: cd desktop-clients\python ^&^& python servlet_client.py
echo.
echo To stop GlassFish later: %ASADMIN% stop-domain
echo ========================================
pause