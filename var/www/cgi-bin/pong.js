// 1. --- THE CGI PART (SERVER-SIDE) ---
// This runs ONCE on the server.
// It prints the HTTP header and the blank line.
console.log("Content-Type: text/html");
console.log(""); // The mandatory blank line

// 2. --- THE HTML/CSS/JS PART (CLIENT-SIDE) ---
// This is the HTML document that gets sent to the browser.
// The browser will then run the <script> part locally.
console.log(`
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Pong</title>
    <style>
        /* Basic CSS for the game */
        body {
            background-color: #121212;
            color: #FFF;
            font-family: 'Courier New', Courier, monospace;
            display: grid;
            place-items: center;
            min-height: 100vh;
            margin: 0;
            overflow: hidden;
        }
        canvas {
            background-color: #000;
            border: 2px solid #FFF;
        }
        h1 {
            color: #00bcd4;
        }
        #controls {
            display: flex;
            gap: 50px;
            font-size: 1.1em;
        }
    </style>
</head>
<body>

    <h1>PONG (Served via CGI)</h1>
    <canvas id="pongCanvas" width="800" height="400"></canvas>
    <div id="controls">
        <p>Player 1: W / S</p>
        <p>Player 2: Up / Down</p>
    </div>

    <script>
        // 3. --- THE GAME LOGIC (CLIENT-SIDE JAVASCRIPT) ---
        // This runs in the user's BROWSER, not on the server.

        const canvas = document.getElementById('pongCanvas');
        const context = canvas.getContext('2d');

        // Game settings
        const paddleWidth = 10;
        const paddleHeight = 100;
        const ballRadius = 10;

        // Player 1 Paddle (Left)
        const player = {
            x: 0,
            y: (canvas.height - paddleHeight) / 2,
            width: paddleWidth,
            height: paddleHeight,
            color: 'WHITE',
            score: 0,
            dy: 8 // speed
        };

        // Player 2 Paddle (Right)
        const com = {
            x: canvas.width - paddleWidth,
            y: (canvas.height - paddleHeight) / 2,
            width: paddleWidth,
            height: paddleHeight,
            color: 'WHITE',
            score: 0,
            dy: 8 // speed
        };

        // The Ball
        const ball = {
            x: canvas.width / 2,
            y: canvas.height / 2,
            radius: ballRadius,
            speed: 7,
            dx: 5, // velocity x
            dy: 5, // velocity y
            color: 'WHITE'
        };

        // --- DRAW FUNCTIONS ---

        function drawRect(x, y, w, h, color) {
            context.fillStyle = color;
            context.fillRect(x, y, w, h);
        }

        function drawCircle(x, y, r, color) {
            context.fillStyle = color;
            context.beginPath();
            context.arc(x, y, r, 0, Math.PI * 2, false);
            context.closePath();
            context.fill();
        }

        function drawText(text, x, y, color) {
            context.fillStyle = color;
            context.font = '75px fantasy';
            context.fillText(text, x, y);
        }

        function drawNet() {
            context.beginPath();
            context.setLineDash([10, 15]);
            context.moveTo(canvas.width / 2, 0);
            context.lineTo(canvas.width / 2, canvas.height);
            context.strokeStyle = 'WHITE';
            context.lineWidth = 5;
            context.stroke();
        }

        // --- GAME LOGIC ---

        // Reset ball to center after a score
        function resetBall() {
            ball.x = canvas.width / 2;
            ball.y = canvas.height / 2;
            ball.speed = 7;
            // Reverse direction
            ball.dx = -ball.dx;
        }

        // Collision detection
        function collision(b, p) {
            // b = ball, p = player
            b.top = b.y - b.radius;
            b.bottom = b.y + b.radius;
            b.left = b.x - b.radius;
            b.right = b.x + b.radius;

            p.top = p.y;
            p.bottom = p.y + p.height;
            p.left = p.x;
            p.right = p.x + p.width;

            return b.right > p.left && b.bottom > p.top && b.left < p.right && b.top < p.bottom;
        }

        // Handle keyboard input
        const keysPressed = {};
        window.addEventListener('keydown', (e) => {
            keysPressed[e.key] = true;
        });
        window.addEventListener('keyup', (e) => {
            keysPressed[e.key] = false;
        });

        // Update function (moves everything)
        function update() {
            // Move paddles based on keysPressed
            // Player 1 (W/S)
            if (keysPressed['w'] && player.y > 0) {
                player.y -= player.dy;
            } else if (keysPressed['s'] && (player.y + player.height) < canvas.height) {
                player.y += player.dy;
            }

            // Player 2 (ArrowUp/ArrowDown)
            if (keysPressed['ArrowUp'] && com.y > 0) {
                com.y -= com.dy;
            } else if (keysPressed['ArrowDown'] && (com.y + com.height) < canvas.height) {
                com.y += com.dy;
            }

            // Move ball
            ball.x += ball.dx;
            ball.y += ball.dy;

            // Ball collision with top/bottom walls
            if (ball.y + ball.radius > canvas.height || ball.y - ball.radius < 0) {
                ball.dy = -ball.dy; // Reverse vertical direction
            }

            // Check for score
            if (ball.x - ball.radius < 0) {
                // Player 2 scores
                com.score++;
                resetBall();
            } else if (ball.x + ball.radius > canvas.width) {
                // Player 1 scores
                player.score++;
                resetBall();
            }

            // Check for paddle collision
            let paddle = (ball.x < canvas.width / 2) ? player : com;

            if (collision(ball, paddle)) {
                // Ball hit a paddle, reverse horizontal direction
                ball.dx = -ball.dx;

                // Optional: increase speed slightly on hit
                // ball.speed += 0.2;
                // ball.dx = (ball.dx > 0 ? 1 : -1) * ball.speed;
            }
        }

        // Render function (draws everything)
        function render() {
            // Clear the canvas
            drawRect(0, 0, canvas.width, canvas.height, 'BLACK');

            // Draw the net
            drawNet();

            // Draw the scores
            drawText(player.score, canvas.width / 4, canvas.height / 5, 'WHITE');
            drawText(com.score, 3 * canvas.width / 4, canvas.height / 5, 'WHITE');

            // Draw the paddles
            drawRect(player.x, player.y, player.width, player.height, player.color);
            drawRect(com.x, com.y, com.width, com.height, com.color);

            // Draw the ball
            drawCircle(ball.x, ball.y, ball.radius, ball.color);
        }

        // The Game Loop
        function gameLoop() {
            update();
            render();
            // Keep the loop going
            requestAnimationFrame(gameLoop);
        }

        // Start the game loop
        requestAnimationFrame(gameLoop);
    </script>
</body>
</html>
`);
