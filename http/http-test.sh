# curl -vX POST http://localhost:8080 \
#   -H "Content-Type: application/x-www-form-urlencoded" \
#   -d "username=testuser&password=123456"

# curl -vX POST http://localhost:8080 \
#   -H "Content-Type: application/json" \
#   -d '{"username":"testuser","password":"123456"}'

# # Basic query parameters
# curl -v "http://localhost:8080/path?param1=value1&param2=value2"

# # Empty values
# curl -v "http://localhost:8080/search?q="

# # Special characters in query parameters
# curl -v "http://localhost:8080/search?q=hello%20world&filter=price%3E100"

# # Multiple parameters with same name
# curl -v "http://localhost:8080/items?tag=new&tag=sale&tag=featured"

# # Different HTTP methods
# curl -v -X PUT "http://localhost:8080/resource"
# curl -v -X DELETE "http://localhost:8080/resource/123"

# # Custom headers with special cases
# curl -v "http://localhost:8080" -H "X-Empty-Header:"
# curl -v "http://localhost:8080" -H "X-Custom-Header: value with spaces"

# # Long URL path
# curl -v "http://localhost:8080/very/long/nested/path/structure/test"

# # Malformed content
# curl -v -X POST "http://localhost:8080" -H "Content-Type: application/json" -d "{malformed json"

# # Large request body
# truncate -s 100M large_file.txt
# curl -v -X POST "http://localhost:8080/upload" \
#   --data-binary @large_file.txt
# rm -rf large_file.txt
