#!/usr/bin/python3

import os
import time

time.sleep(10)

# Required header
print("Status: 201")
print("Content-Type: text/html")
# Optional: The server will calculate this if you don't
# print("Content-Length: 55")
print() # This blank line is CRITICAL

# Body
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("</body>")
print("</html>")
