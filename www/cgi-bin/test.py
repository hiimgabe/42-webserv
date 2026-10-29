
import os
import sys
import html
from urllib.parse import parse_qs

#sys.stderr = open("/dev/pts/4", "w")

contentLength = int(os.environ.get("CONTENT_LENGTH", 0))
#print(f"CONT_LEN: {contentLength}", file=sys.stderr)

postData = sys.stdin.read(contentLength)
#print(f"Actual payload size: {len(postData)}", file=sys.stderr)

params = parse_qs(postData)
msg = params.get("msg", [""])[0]

msg = html.escape(msg)

responseBody = f"""
<!DOCTYPE html>
<html>
	<head>
		<link rel="stylesheet" href="/assets/css/main.css">
		<link rel="stylesheet" href="/assets/css/header.css">
		<title>Home</title>
	</head>
	<body>
		<div id="header">
		</div>
		<div id="container">
	  	<h1>POST some text</h1>
	  	<form action="/cgi-bin/test.py" method="POST">
	  		<label for="msg">Message:</label>
	  		<input type="text" name="msg" id="msg">
	  		<input type="submit" value="Send">
	  	</form>




	  		<h3>Message:</h3>
	  		<p style="word-wrap: break-word;">{msg}</p>
		</div>
	</body>
</html>
"""



print("Content-Type: text/html; charset=UTF-8\r\n\r\n")
print(responseBody)