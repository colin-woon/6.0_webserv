#!/bin/bash

# Configuration
PORT=8081

# Function to send HTTP request via netcat
send_request() {
    echo -en "$1" | nc -N localhost $PORT
}

# Redirect output to file
# OUTPUT_FILE="http_results.txt"
# echo "Saving results to $OUTPUT_FILE"
# echo "==========================================" > $OUTPUT_FILE

# # Large request body example (uncomment to use)
# echo "TEST 19: large request body."
# echo "EXPECTED: Large payload handling"
# echo "----------------------------------------"
# truncate -s 100M large_file.txt
# cat large_file.txt | nc localhost $PORT
# rm -rf large_file.txt
# echo ""

# # POST with form-urlencoded data
# echo "TEST 1: POST with form-urlencoded data."
# echo "EXPECTED: Normal POST request handling"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 23\r\n\r\nusername=testuser&password=123456"
# echo ""

# # POST with JSON data
# echo "TEST 2: POST with JSON data."
# echo "EXPECTED: Normal POST request handling"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 37\r\n\r\n{\"username\":\"testuser\",\"password\":\"123456\"}"
# echo ""

# # GET with query parameters
# echo "TEST 3: GET with query parameters."
# echo "EXPECTED: Normal GET request handling"
# echo "----------------------------------------"
# send_request "GET /path?param1=value1&param2=value2 HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # GET with empty query parameter
# echo "TEST 4: GET with empty query parameter."
# echo "EXPECTED: Normal GET request handling"
# echo "----------------------------------------"
# send_request "GET /search?q= HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # Special characters in query parameters (example)
# echo "TEST 5: GET with special characters in query parameters."
# echo "EXPECTED: Normal GET request handling"
# echo "----------------------------------------"
# send_request "GET /search?q=hello%20world&filter=price%3E100 HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # Multiple parameters with same name (example)
# echo "TEST 6: GET with multiple parameters with same name."
# echo "EXPECTED: Normal GET request handling"
# echo "----------------------------------------"
# send_request "GET /items?tag=new&tag=sale&tag=featured HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # Different HTTP methods (examples)
# echo "TEST 7: PUT request."
# echo "EXPECTED: Normal PUT request handling"
# echo "----------------------------------------"
# send_request "PUT /resource HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 8: DELETE request."
# echo "EXPECTED: Normal DELETE request handling"
# echo "----------------------------------------"
# send_request "DELETE /resource/123 HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # Custom headers with special cases (examples)
# echo "TEST 9: request with custom headers."
# echo "EXPECTED: Normal request handling with custom headers"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nX-Empty-Header:\r\nX-Custom-Header: value with spaces\r\n\r\n"
# echo ""

# # Long URL path (example)
# echo "TEST 10: request with long URL path."
# echo "EXPECTED: Normal GET request handling"
# echo "----------------------------------------"
# send_request "GET /very/long/nested/path/structure/test HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # Malformed content example (warning: this will likely fail)
# echo "TEST 11: malformed JSON request."
# echo "EXPECTED: 400 Bad Request or similar error"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{malformed json"
# echo ""

# echo "TEST 12: LF-only line terminators."
# Line terminator tests
# echo "TEST 12: LF-only line terminators."
# echo "EXPECTED: Acceptable normal behaviour"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 12.1: multiple line terminators."
# echo "EXPECTED: Acceptable normal behaviour"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\nHost: localhost\r\n\r\n\r\n"
# echo ""

# echo "TEST 13: CR not followed by LF within protocol elements."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\rHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 13.1: CR not followed by LF within protocol elements."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\rContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 13.2: Multiple CR within protocol elements."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 13.3: Multiple CR within headers."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 13.5: Headers without Host."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\n\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 13.6: Headers with more than one Host."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"test\":\"value\"}"
# echo ""

# # Empty line before request line test
# echo "TEST 14: empty line before request line."
# echo "EXPECTED: Acceptable normal behaviour"
# echo "----------------------------------------"
# send_request "\r\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 14.1: empty line before request line."
# echo "EXPECTED: Acceptable normal behaviour"
# echo "----------------------------------------"
# send_request "\n\n\nGET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# # Whitespace between startline and header field (should be rejected)
# echo "TEST 15: whitespace between startline and header field."
# echo "EXPECTED: 400 Bad Request (rejected to prevent smuggling)"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\n Host: localhost\r\n\r\n"
# echo ""

# echo "TEST 15.1: Missing Content-Length and Transfer-Encoding with body content"
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 15.2: Both Content-Length and Transfer-Encoding present"
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 20\r\nTransfer-Encoding: chunked\r\n\r\n{\"test\":\"value\"}"
# echo ""

# echo "TEST 15.3: CR in body content"
# echo "EXPECTED: 200 OK (or server-defined success response)"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/json\r\nContent-Length: 22\r\n\r\n{\"test\":\"value\rwithCR\"}"
# echo ""

# # Grammar error test (should return 400 Bad Request)
# echo "TEST 16.0: grammar error - GET"
# echo "EXPECTED: 501 Not Implemented"
# echo "----------------------------------------"
# send_request "GTE / HTTP/1.2\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 16.1: grammar error, method longer than 10"
# echo "EXPECTED: 501 Not Implemented"
# echo "----------------------------------------"
# send_request "GEEEEEEEEEEEEET / HTTP/1.2\r\nHost: localhost\r\n\r\n"
# echo ""

# # Grammar error test (should return 400 Bad Request)
# echo "TEST 16.2: grammar error lowercase"
# echo "EXPECTED: 400 Bad Request, Connection: closed"
# echo "----------------------------------------"
# send_request "PzSOTW / HTTP/1.2\r\nHost: localhost\r\n\r\n"
# # echo ""

# echo "TEST 16.3: URI Too Long"
# echo "EXPECTED: 414 URI Too Long"
# echo "----------------------------------------"
# # Generate a long URI by repeating '/' 8001 times (exceeding MAX_URI_LENGTH of 8000)
# long_uri=$(printf '/%.0s' $(seq 1 8001))
# send_request "GET $long_uri HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 17: request line no target"
# echo "EXPECTED: 400 Bad Request, Connection: closed"
# echo "----------------------------------------"
# send_request "GET  HTTP/1.2\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 17.1: request line no version"
# echo "EXPECTED: 400 Bad Request, Connection: closed"
# echo "----------------------------------------"
# send_request "GET /\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 17.2: request line no method"
# echo "EXPECTED: 400 Bad Request, Connection: closed"
# echo "----------------------------------------"
# send_request "/ HTTP/1.1r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 17.3: request line nothing"
# echo "EXPECTED: 400 Bad Request, Connection: closed"
# echo "----------------------------------------"
# send_request "r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 18 HTTP Version"
# echo "EXPECTED: 505 Http Version Not Supported, Connection: closed"
# echo "----------------------------------------"
# send_request "GET / HPPT/1.2\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 19: Request target with multiple whitespace."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET /path   with   multiple   spaces HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

# echo "TEST 23.0: Request with a header value that is CR"
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nCustom-Header: \r\r\n\r\n"
# echo ""

# echo "TEST 23.1: Request with a header value that is LF"
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost: \n\nCustom-Header: \r\n\r\n"
# echo ""

# echo "TEST 23.2a: Request with a header value that is NUL."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost: localhost\r\nCustom-Header: \x00\x00\x00\r\n\r\n"
# echo ""

# echo "TEST 23.2b: Request with a header value that contains NUL."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost: loc\x00alhost\r\nCustom-Header: \r\n\r\n"
# echo ""

# echo "TEST 23.2c: Request line with NUL."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\x00\r\nHost: localhost\r\nCustom-Header: \r\n\r\n"
# echo ""

# echo "TEST 24.0: Request with a header value that has trailing and leading whitespaces SPACE."
# echo "EXPECTED: Strip whitespace and parse normally"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost:  localhost  \r\n\r\n"
# echo ""

# echo "TEST 24.1: Request with a header value that has trailing and leading whitespaces. HTAB"
# echo "EXPECTED: Strip whitespace and parse normally"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost:		localhost		\r\n\r\n"
# echo ""

# echo "TEST 25: Request with a header name and colon separated by any whitespace."
# echo "EXPECTED: 400 Bad Request"
# echo "----------------------------------------"
# send_request "GET / HTTP/1.1\r\nHost : localhost\r\n\r\n"
# echo ""

# echo "TEST 26: Request without both Content-Length or Transfer-Encoding header but with a body."
# echo "EXPECTED: 400 Bad Request, more robust"
# echo "----------------------------------------"
# send_request "POST / HTTP/1.1\r\nHost: localhost\r\n\r\n{\"test\":\"value\"}"
# echo ""

# # DELETE Unallowed
# echo "TEST 27: Unallowed DELETE."
# echo "EXPECTED: 405 Method Not Allowed"
# echo "----------------------------------------"
# send_request "DELETE / HTTP/1.1\r\nHost: localhost\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 0\r\n\r\n"
# echo ""

# DELETE
# echo "TEST 28: DELETE."
# echo "EXPECTED: 204 No Content"
# echo "----------------------------------------"
# send_request "DELETE /uploads/d1d86da2a930e50f.pdf HTTP/1.1\r\nHost: localhost\r\n\r\n"
# echo ""

echo "TEST 29: Overflow Header Cap from Config (min 512 bytes, need to setup in config)."
echo "EXPECTED: 431 Request Header Fields Too Large"
echo "----------------------------------------"
curl -v -H "X-Overflow: $(printf '%500s' | tr ' ' 'A')" http://127.0.0.1:8081/
echo ""

# echo "=========================================="
# echo "Test completed. Results saved to $OUTPUT_FILE"

# echo "Results saved to $OUTPUT_FILE"
