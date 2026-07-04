package ro.uaic.dbxdrgsl.servletdemo;

import jakarta.servlet.*;
import jakarta.servlet.http.*;
import jakarta.servlet.annotation.*;
import java.io.IOException;
import java.util.Collections;
import java.util.Locale;
import java.util.logging.Logger;

/**
 * Handles POST requests from index.jsp and routes to page1 or page2.
 */
@WebServlet(name = "RouteServlet", urlPatterns = {"/route"})
public class RouteServlet extends HttpServlet {

    private static final Logger log = Logger.getLogger(RouteServlet.class.getName());

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        // 1. Get the selected choice value
        String choice = request.getParameter("choice");
        String destination;
        
        if ("1".equals(choice)) {
            destination = "/page1.html";
        } else if ("2".equals(choice)) {
            destination = "/page2.html";
        } else {
            destination = "/index.jsp";  // default fallback
        }

        // 2. Log request information
        String method = request.getMethod();
        String ip = request.getRemoteAddr();
        String ua = request.getHeader("User-Agent");
        String langs = Collections.list(request.getLocales())
                                  .stream()
                                  .map(Locale::toLanguageTag)
                                  .reduce((a,b)->a + "," + b)
                                  .orElse("");
        
        log.info(() -> String.format("Request: method=%s, ip=%s, user-agent=%s, languages=%s, parameter=%s, destination=%s",
                method, ip, ua, langs, choice, destination));

        // 3. Forward the request to the correct page
        request.getRequestDispatcher(destination).forward(request, response);
    }
}