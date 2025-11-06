#!/usr/bin/env python3

import sys
import time

# CGI headers
print("Status: 200 OK")
print("Content-Type: text/html\n")

# Simulate large output and slow streaming
print("<html><body>")
print("<h1>Large CGI Output Test</h1>")
print("<p>This script generates >5000 bytes of data to force multiple reads.</p>")

# Create a large chunk of content
for i in range(100):  # 100 * 100 = 10,000 lines approx
    print(f"<p>Line {i:04d}: " + "x" * 50 + "</p>")
    sys.stdout.flush()
    time.sleep(0.01)  # Add delay to test partial read handling

print("</body></html>")
