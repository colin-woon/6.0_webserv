#!/bin/bash

# Configuration
PORT=8002

# Function to send HTTP request via netcat
send_request() {
    echo -en "$1" | nc localhost $PORT
}

# Redirect output to file
OUTPUT_FILE="http_results.txt"
echo "Saving results to $OUTPUT_FILE"
echo "==========================================" > $OUTPUT_FILE

# Large request body example (uncomment to use)
# echo "TEST 19: large request body." >> $OUTPUT_FILE
# echo "EXPECTED: Large payload handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# truncate -s 100M large_file.txt
# cat large_file.txt | nc localhost $PORT >> $OUTPUT_FILE
# rm -rf large_file.txt
# echo "" >> $OUTPUT_FILE

# # POST with form-urlencoded data
# echo "TEST 1: POST with form-urlencoded data." >> $OUTPUT_FILE
# echo "EXPECTED: Normal POST request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 23\r\n\r\nusername=testuser&password=123456" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # POST with JSON data
# echo "TEST 2: POST with JSON data." >> $OUTPUT_FILE
# echo "EXPECTED: Normal POST request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 37\r\n\r\n{\"username\":\"testuser\",\"password\":\"123456\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # GET with query parameters
# echo "TEST 3: GET with query parameters." >> $OUTPUT_FILE
# echo "EXPECTED: Normal GET request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET /path?param1=value1&param2=value2 HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # GET with empty query parameter
# echo "TEST 4: GET with empty query parameter." >> $OUTPUT_FILE
# echo "EXPECTED: Normal GET request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET /search?q= HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Special characters in query parameters (example)
# echo "TEST 5: GET with special characters in query parameters." >> $OUTPUT_FILE
# echo "EXPECTED: Normal GET request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET /search?q=hello%20world&filter=price%3E100 HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Multiple parameters with same name (example)
# echo "TEST 6: GET with multiple parameters with same name." >> $OUTPUT_FILE
# echo "EXPECTED: Normal GET request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET /items?tag=new&tag=sale&tag=featured HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Different HTTP methods (examples)
# echo "TEST 7: PUT request." >> $OUTPUT_FILE
# echo "EXPECTED: Normal PUT request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "PUT /resource HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 8: DELETE request." >> $OUTPUT_FILE
# echo "EXPECTED: Normal DELETE request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "DELETE /resource/123 HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Custom headers with special cases (examples)
# echo "TEST 9: request with custom headers." >> $OUTPUT_FILE
# echo "EXPECTED: Normal request handling with custom headers" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nX-Empty-Header:\r\nX-Custom-Header: value with spaces\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Long URL path (example)
# echo "TEST 10: request with long URL path." >> $OUTPUT_FILE
# echo "EXPECTED: Normal GET request handling" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET /very/long/nested/path/structure/test HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Malformed content example (warning: this will likely fail)
# echo "TEST 11: malformed JSON request." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request or similar error" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{malformed json" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 12: LF-only line terminators." >> $OUTPUT_FILE
# Line terminator tests
# echo "TEST 12: LF-only line terminators." >> $OUTPUT_FILE
# echo "EXPECTED: Acceptable normal behaviour" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 13: CR not followed by LF within protocol elements." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\rHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 13.1: CR not followed by LF within protocol elements." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\rContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 13.2: Multiple CR within protocol elements." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 13.3: Multiple CR within headers." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

echo "TEST 13.5: Headers without Host." >> $OUTPUT_FILE
echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
echo "----------------------------------------" >> $OUTPUT_FILE
send_request "GET / HTTP/1.1\r\n\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
echo "" >> $OUTPUT_FILE

# echo "TEST 13.6: Headers with more than one Host." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE


# # Empty line before request line test
# echo "TEST 14: empty line before request line." >> $OUTPUT_FILE
# echo "EXPECTED: Acceptable normal behaviour" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "\r\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 14.1: empty line before request line." >> $OUTPUT_FILE
# echo "EXPECTED: Acceptable normal behaviour" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "\n\n\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Whitespace between startline and header field (should be rejected)
# echo "TEST 15: whitespace between startline and header field." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request (rejected to prevent smuggling)" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\n Host: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 13.2: CR not followed by LF within body content." >> $OUTPUT_FILE
# echo "EXPECTED: Acceptable normal behaviour" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"\r}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE


# echo "TEST 13.4: Body without Content-Length or Encoding." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Grammar error test (should return 400 Bad Request)
# echo "TEST 16.0: grammar error - GET" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request, Connection: closed" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GTE / HTTP/1.2\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Grammar error test (should return 400 Bad Request)
# echo "TEST 16.1: grammar error - POST" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request, Connection: closed" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "PSOTW / HTTP/1.2\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Grammar error test (should return 400 Bad Request)
# echo "TEST 16.3: grammar error - DELETE" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request, Connection: closed" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "D / HTTP/1.2\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 16.4: grammar error - HTTP" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request, Connection: closed" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HPPT/1.2\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 16.5: grammar error - HTTP version" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request, Connection: closed" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.2\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Method longer than implemented test (should return 501 Not Implemented)
# echo "TEST 17: method longer than 10 octets." >> $OUTPUT_FILE
# echo "EXPECTED: 501 Not Implemented status code" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "THIS_IS_A_VERY_LONG_METHOD_NAME_THAT_EXCEEDS_TEN_OCTETS / HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# # Request-line longer than 8000 octets test (should return 414 URI Too Long)
# echo "TEST 18: request-target longer than 8000 octets." >> $OUTPUT_FILE
# echo "EXPECTED: 414 URI Too Long status code" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# long_uri=$(printf '/%.0s' $(seq 1 7985))
# send_request "GET $long_uri HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 20: Request target with multiple whitespace." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET /path   with   multiple   spaces HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 21.0: Request with missing Host header" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 21.1: Request with multiple Host headers." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\n\r\nHost: localhost\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 22: Empty request target." >> $OUTPUT_FILE
# echo "EXPECTED: Redirect to default / (root) directory" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET  HTTP/1.1\r\nHost: localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 23.0: Request with a header value that is CR" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nCustom-Header: \r\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 23.1: Request with a header value that is LF" >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nCustom-Header: \n\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 23.2: Request with a header value that is NUL." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nCustom-Header: \x00\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 24.0: Request with a header value that has trailing and leading whitespaces SPACE." >> $OUTPUT_FILE
# echo "EXPECTED: Strip whitespace and parse normally" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost:  localhost  \r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 24.1: Request with a header value that has trailing and leading whitespaces. HTAB" >> $OUTPUT_FILE
# echo "EXPECTED: Strip whitespace and parse normally" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost:		localhost		\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 25: Request with a header name and colon separated by any whitespace." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "GET / HTTP/1.1\r\nHost : localhost\r\n\r\n" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

# echo "TEST 26: Request without both Content-Length or Transfer-Encoding header but with a body." >> $OUTPUT_FILE
# echo "EXPECTED: 400 Bad Request, more robust" >> $OUTPUT_FILE
# echo "----------------------------------------" >> $OUTPUT_FILE
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\n\r\n{\"test\":\"value\"}" >> $OUTPUT_FILE
# echo "" >> $OUTPUT_FILE

echo "==========================================" >> $OUTPUT_FILE
echo "Test completed. Results saved to $OUTPUT_FILE" >> $OUTPUT_FILE

echo "Results saved to $OUTPUT_FILE"
