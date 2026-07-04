# Lab 01 — Java Servlet Technology (2025-2026) — Solution Summary

This README explains exactly how the provided solution meets the lab requirements. It contains the actual project layout, the exact asadmin commands taken from the repository scripts, a line-by-line annotation of ApiChooseServlet.java, and a short "Further implementation to be done" section describing using JSF and real-world applicability.

Project layout (actual files)
- servlet-demo/src/main/java/ro/uaic/dbxdrgsl/servletdemo/ApiChooseServlet.java
- servlet-demo/src/main/java/ro/uaic/dbxdrgsl/servletdemo/RouteServlet.java
- servlet-demo/src/main/webapp/index.jsp
- servlet-demo/src/main/webapp/page1.html
- servlet-demo/src/main/webapp/page2.html
- servlet-demo/pom.xml
- servlet-demo/desktop-clients/java/ServletClient.java
- servlet-demo/desktop-clients/python/servlet_client.py
- helper scripts: servlet-demo/setup-and-deploy.bat, servlet-demo/deploy-glassfish.bat

Project creation (maven archetype)
- Command used to generate the skeleton:
  ```
  mvn archetype:generate \
    -DarchetypeArtifactId=maven-archetype-webapp \
    -DgroupId=ro.uaic.dbxdrgsl \
    -DartifactId=servlet-demo \
    -Dversion=1.0-SNAPSHOT \
    -DinteractiveMode=false
  ```
- Short meanings:
  - groupId: reverse-domain identifier for Java package namespace (ro.uaic.dbxdrgsl).
  - artifactId: project/artifact name (servlet-demo) — becomes the WAR name and folder.
  - version: artifact version used by Maven.

Build & deploy (exact asadmin commands from repository scripts)
- Build WAR (from workspace root):
  ```
  mvn -f servlet-demo/pom.xml clean package
  ```
  Output: `servlet-demo/target/servlet-demo.war`

- Commands extracted verbatim from setup-and-deploy.bat (Windows paths as in repo):
  ```
  "%GLASSFISH_HOME%\bin\asadmin.bat" start-domain
  "%GLASSFISH_HOME%\bin\asadmin.bat" list-applications
  "%GLASSFISH_HOME%\bin\asadmin.bat" undeploy servlet-demo 2>nul
  "%GLASSFISH_HOME%\bin\asadmin.bat" deploy target\servlet-demo.war
  "%GLASSFISH_HOME%\bin\asladmin.bat" list-applications
  ```
  (script sets `GLASSFISH_HOME=C:\Users\Dragos\OneDrive\Code\JavaTech\glassfish7` and `JAVA_HOME=C:\Program Files\Java\jdk-25`.)

- Commands extracted verbatim from deploy-glassfish.bat (Windows paths as in repo):
  ```
  call "%ASADMIN%" start-domain
  call "%ASLADMIN%" undeploy servlet-demo 2>nul
  call "%ASADMIN%" deploy target\servlet-demo.war
  call "%ASADMIN%" list-applications
  ```
  (script sets `ASADMIN=%GLASSFISH_HOME%\bin\asadmin.bat` and `JAVA_HOME=C:\Program Files\Java\jdk-25`.)

- Common (Linux / container) equivalents you can run interactively:
  ```
  # start domain (default domain name)
  $GLASSFISH_HOME/bin/asadmin start-domain

  # deploy with context root 'servlet-demo'
  $GLASSFISH_HOME/bin/asadmin deploy --contextroot servlet-demo servlet-demo/target/servlet-demo.war

  # redeploy (preferred)
  $GLASSFISH_HOME/bin/asadmin redeploy --name servlet-demo servlet-demo/target/servlet-demo.war

  # force deploy (undeploy+deploy)
  $GLASSFISH_HOME/bin/asadmin deploy --force=true --contextroot servlet-demo servlet-demo/target/servlet-demo.war

  # undeploy explicitly
  $GLASSFISH_HOME/bin/asadmin undeploy servlet-demo

  # list deployed apps / domains
  $GLASSFISH_HOME/bin/asadmin list-applications
  $GLASSFISH_HOME/bin/asadmin list-domains

  # stop domain
  $GLASSFISH_HOME/bin/asadmin stop-domain
  ```

Open the app and inspect logs
- Browser: "$BROWSER" http://localhost:8080/servlet-demo/
- API quick-check: http://localhost:8080/servlet-demo/api/choose?choice=1
- GlassFish admin console: http://localhost:4848
- Tail server log (Linux):
  ```
  tail -f $GLASSFISH_HOME/glassfish/domains/domain1/logs/server.log
  ```

What ?choice=... means (brief, crucial)
- "choice" is a request parameter. It can appear:
  - In the query string: /api/choose?choice=2 (GET)
  - In the request body for POST with content-type application/x-www-form-urlencoded (form submission)
- Servlet API: `req.getParameter("choice")` returns the parameter regardless of GET or POST for typical form/urlencoded requests.
- In this project, the form on index.jsp sends a parameter named "choice" with value "1" or "2". The controller reads it and either forwards the browser to page1.html/page2.html or, for API/desktop clients, returns the parameter value as plain text.

Relevant snippets (actual files in repo)
- index.jsp (form that sends `choice`):
```jsp
<!-- filepath: servlet-demo/src/main/webapp/index.jsp -->
<%@ page contentType="text/html;charset=UTF-8" language="java" %>
<!doctype html>
<html>
<head><meta charset="utf-8"><title>Welcome</title></head>
<body>
  <h1>Welcome</h1>
  <form method="post" action="${pageContext.request.contextPath}/route">
    <label><input type="radio" name="choice" value="1" checked> Page 1</label><br>
    <label><input type="radio" name="choice" value="2"> Page 2</label><br>
    <button type="submit">Go</button>
  </form>
</body>
</html>
```

- ApiChooseServlet.java (actual file) and line-by-line annotation
```java
// filepath: servlet-demo/src/main/java/ro/uaic/dbxdrgsl/servletdemo/ApiChooseServlet.java
package ro.uaic.dbxdrgsl.servletdemo;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.*;
import java.io.IOException;

@WebServlet(name="ApiChoose", urlPatterns={"/api/choose"})
public class ApiChooseServlet extends HttpServlet {
  @Override protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws IOException {
    // accept either ?choice=1 or buttons named page1/page2
    String choice = req.getParameter("choice");
    if (choice == null) {
      if (req.getParameter("page1") != null) choice = "1";
      else if (req.getParameter("page2") != null) choice = "2";
    }
    if (choice == null) choice = "";
    resp.setContentType("text/plain;charset=UTF-8");
    resp.getWriter().print(choice);
  }

  @Override protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws IOException {
    doPost(req, resp); // allow GET for quick tests
  }
}
```
Annotation (line-by-line, concise)
- `@WebServlet(name="ApiChoose", urlPatterns={"/api/choose"})` — maps this servlet to /api/choose.
- `doPost(...)` — handles POST form submissions (and non-browser clients posting urlencoded data).
- `String choice = req.getParameter("choice");` — reads "choice" parameter whether it came via query string or form body.
- Fallback: if `choice` is null, check for buttons named `page1` or `page2` (HTML forms may submit named buttons).
- `if (choice == null) choice = "";` — normalize to empty string instead of null for response.
- `resp.setContentType("text/plain;charset=UTF-8");` — ensures plain-text response as required for desktop/API clients.
- `resp.getWriter().print(choice);` — returns the raw parameter value.
- `doGet(...)` delegates to `doPost(...)` to allow quick manual GET tests like /api/choose?choice=1.

How the backend handles both browser forwarding and desktop/API callers
- RouteServlet (controller) forwards browser requests to static pages when a browser submits the form.
- ApiChooseServlet (API endpoint) returns plain text and is tolerant of query-string GET or form POST.
- Desktop clients (examples in repo) set Accept: text/plain or call the API path /api/choose to receive plain text:
  - curl:
    ```
    curl -s -X POST http://localhost:8080/servlet-demo/api/choose -d "choice=2"
    # prints: 2
    ```
  - Java (desktop-clients/java/ServletClient.java) uses java.net.http.HttpClient posting urlencoded body and printing response.
  - Python (desktop-clients/python/servlet_client.py) uses requests.post(..., data={"choice":"1"}) and prints text.

Server logging (what to expect)
- RouteServlet logs: HTTP method, client IP, User-Agent, Accept-Language and the `choice` parameter on every request. Check server.log for lines similar to:
  ```
  INFO: method=POST; ip=127.0.0.1; user-agent=Java/17+HttpClient; accept-language=en-US,en;q=0.5; choice=2
  ```

Further implementation to be done (JSF, polishing, real-world usage)
- If you want to evolve this prototype into a producible web app, consider:
  - Using JSF (JavaServer Faces) for component-driven UI:
    - JSF provides page composition, component lifecycle, validation, and navigation rules that simplify forms, server-side validation, and richer UI interactions.
    - Replace index.jsp with a JSF page (index.xhtml) and bind radio selection to a backing bean; navigation can be handled declaratively or via action methods.
    - Benefit: better separation of concerns, built-in converters/validators, integration with CDI for business logic.
  - Adding templating and CSS frameworks (Bootstrap) for consistent UI and responsive design.
  - Adding input validation and error pages, session handling, and security (HTTPS, authentication).
  - Packaging and CI/CD:
    - Add automated build pipelines (GitHub Actions) to build the WAR and run integration tests.
    - Use asadmin or vendor-specific tooling in deployment steps.
  - Persistence and business logic:
    - For simple logic like choosing pages, the pattern is used in many small control panels, surveys, health-check endpoints, or feature toggles.
    - With polishing (persistence, multi-step flows, user accounts) such simple flows become parts of admin dashboards, onboarding wizards, or microservices UI for system status.
- Types of web applications that start with this logic:
  - Admin consoles and dashboards (step/choice routing).
  - Simple form-driven microapps (surveys, toggles, thin APIs).
  - Prototypes/PoCs that later grow into multi-page JSF/Thymeleaf applications with database-backed state.

If you want I will:
- Insert the exact Windows asadmin command block from servlet-demo/setup-and-deploy.bat and deploy-glassfish.bat verbatim into the README (already included above).
- Expand the ApiChooseServlet annotation into a line-numbered walkthrough.
- Add a short checklist and copyable commands section for Ubuntu dev container usage.