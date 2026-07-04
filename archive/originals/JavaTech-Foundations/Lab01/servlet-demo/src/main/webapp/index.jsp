<!DOCTYPE html>
<html>
<head>
    <title>Servlet Demo - Welcome</title>
</head>
<body>
    <h1>Welcome to Servlet Demo Application</h1>
    <p>Please select a page to navigate to:</p>
    
    <form action="route" method="post">
        <div>
            <input type="radio" id="page1" name="choice" value="1" checked>
            <label for="page1">Page 1</label>
        </div>
        <div>
            <input type="radio" id="page2" name="choice" value="2">
            <label for="page2">Page 2</label>
        </div>
        <br>
        <input type="submit" value="Navigate">
    </form>
</body>
</html>
