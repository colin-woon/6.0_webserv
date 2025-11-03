#!/usr/bin/python3

import os
import time

time.sleep(20)

# Required header
print("Content-Type: text/html")
# Optional: The server will calculate this if you don't
# print("Content-Length: 55")
print("\r\n\r\n") # This blank line is CRITICAL

# Body
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello from Python CGI!</h1>")
print("</body>")
print("</html>")
