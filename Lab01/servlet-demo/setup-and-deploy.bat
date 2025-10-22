@echo off
REM GlassFish Setup and Deployment Helper
REM This script sets up the environment and deploys the servlet application

echo ========================================
echo GlassFish Setup and Deployment
echo ========================================
echo.

REM Set GlassFish location
set GLASSFISH_HOME=C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7

REM Set Java home
set JAVA_HOME=C:\Program Files\Java\jdk-25
set JAVA_EXE=%JAVA_HOME%\bin\java.exe

echo Java home: %JAVA_HOME%
echo Java executable: %JAVA_EXE%

REM Verify Java installation
if not exist "%JAVA_EXE%" (
    echo ERROR: Java not found at %JAVA_EXE%
    pause
    exit /b 1
)
echo GlassFish home: %GLASSFISH_HOME%
echo.

REM Set environment for this session
set PATH=%JAVA_HOME%\bin;%PATH%

echo 1. Building the application...
call mvn clean package
if %ERRORLEVEL% neq 0 (
    echo ERROR: Maven build failed!
    pause
    exit /b 1
)
echo ✓ Application built successfully
echo.

echo 2. Starting GlassFish domain...
cd /d "%GLASSFISH_HOME%"
call bin\asadmin.bat start-domain
echo.

echo 3. Checking if application is already deployed...
call bin\asadmin.bat list-applications
echo.

echo 4. Undeploying previous version (if exists)...
call bin\asadmin.bat undeploy servlet-demo 2>nul
echo Previous version undeployed (if existed)
echo.

echo 5. Deploying new version...
cd /d "C:\Users\Dragos\OneDrive\Code\JavaTech\servlet-demo"
call "%GLASSFISH_HOME%\bin\asadmin.bat" deploy target\servlet-demo.war
if %ERRORLEVEL% neq 0 (
    echo ERROR: Deployment failed!
    pause
    exit /b 1
)
echo ✓ Application deployed successfully
echo.

echo 6. Verifying deployment...
call "%GLASSFISH_HOME%\bin\asladmin.bat" list-applications
echo.

echo ========================================
echo DEPLOYMENT SUCCESSFUL!
echo ========================================
echo.
echo ✓ GlassFish is running
echo ✓ servlet-demo application is deployed
echo.
echo Access points:
echo - Web Application: http://localhost:8080/servlet-demo
echo - API Endpoint: http://localhost:8080/servlet-demo/api/choose?choice=1
echo - Admin Console: http://localhost:4848
echo.
echo Desktop client testing:
echo - Java: cd desktop-clients\java ^&^& java ServletClient
echo - Python: cd desktop-clients\python ^&^& python servlet_client.py
echo.
echo To stop GlassFish: "%GLASSFISH_HOME%\bin\asadmin.bat" stop-domain
echo ========================================

REM Open browser to the application
echo Opening application in default browser...
start http://localhost:8080/servlet-demo

pause