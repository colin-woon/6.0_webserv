#!/usr/bin/php-cgi
<?php

// 1. --- THE HTML GENERATOR FUNCTIONS (THE LOGIC) ---

/**
 * Generates the top part of an HTML5 document.
 * @param string $title The title of the page.
 * @return string The HTML header.
 */
function generate_header($title = "Default Title") {
    // HEREDOC syntax (<<<HTML) is a clean way to write large strings in PHP.
    return <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>$title</title>
    <style>
        body { font-family: sans-serif; background: #333; color: #EEE; }
        .container { max-width: 800px; margin: 20px auto; padding: 20px; background: #444; border-radius: 8px; }
        h1 { color: #00bcd4; }
    </style>
</head>
<body>
<div class="container">
HTML;
}

/**
 * Generates the closing part of an HTML5 document.
 * @return string The HTML footer.
 */
function generate_footer() {
    $year = date('Y');
    return <<<HTML
</div>
<footer>
    <p>&copy; $year Gigachad Industries</p>
</footer>
</body>
</html>
HTML;
}


// 2. --- THE "BUSINESS LOGIC" (THE DATA) ---

$page_title = "My PHP Generator";
$user_name = "Gigachad";
$content = "<h1>Welcome, $user_name!</h1><p>This HTML was built by PHP functions. This is much cleaner than mixing code and HTML.</p>";


// 3. --- THE CGI OUTPUT (THE "VIEW") ---

// First, print the mandatory CGI headers
echo "Status: 200 OK\n";
echo "Content-Type: text/html\n";
echo "\n"; // The critical blank line

// Now, echo the generated HTML parts in order
echo generate_header($page_title);
echo $content; // Echo the main body content
echo generate_footer();

?>
