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
