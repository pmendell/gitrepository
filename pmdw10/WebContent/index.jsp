<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
    pageEncoding="ISO-8859-1"%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title>Insert title here</title>
</head>
<body>
<h2>THis is #10</h2>

<jsp:useBean id="myBean" class="pmdw10.TestBean"> </jsp:useBean>

<p>Message: 
   <jsp:getProperty name="myBean" property="message"/>
</p>
</body>
</html>