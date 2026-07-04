Java Technologies - Lab 1
[valid 2025-2026]

Java Servlet Technology

Compulsory (1p)

     Install a Java/Jakarta EE server: Apache Tomcat, Eclipse GlassFish, or Payara Server.
     Create a Java EE application, deploy it on the server. Verify you can access the welcome page.



Homework (2p)

     Create two simple HTML pages in the application, page1.html and page2.html.
     The application will have a dynamically generated welcome page, containing a form that lets the user
     select either the value 1 or 2.
     The input will be submitted to a controller servlet that will forward the request either to page1 or page2,
     depending on the selected value.
     Write in the server log the following information about each request: the HTTP method used, the IP-
     address of the client, the user-agent, the client language(s) and the parameter of the request.
     Invoke the servlet from a desktop application (Java, Python, .NET, etc.).
     The response of the servlet will be, in this case, a plain text containing the value of the parameter, and not
     the content of a page.


Resources

     Java EE Specification APIs / Jakarta EE Platform API
     The Java EE Tutorial / The Jakarta EE Tutorial
     Payara Server
     Java Servlet Technology
     java.net.http.HttpClient
