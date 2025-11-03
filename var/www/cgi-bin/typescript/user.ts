#!/usr/bin/env ts-node-script --swc

/**
 * A CGI script (using ts-node) for a simple User API.
 * - GET: Responds to /user.ts?id=1 (returns JSON)
 * - POST: Responds to form data (application/x-www-form-urlencoded)
 * - OUTPUT: Always returns JSON.
 */

// --- Mock Database (lives for one request, then dies) ---
// In a real app, this would read/write to a file or external DB.
const db = new Map<string, { id: string; name: string; title: string }>();
// Pre-populate our "database"
db.set("1", { id: "1", name: "Gigachad", title: "CEO" });
db.set("2", { id: "2", name: "Colin", title: "Dev" });

/**
 * Asynchronously reads the full request body from stdin.
 *
 * HACK: This version does NOT wait for 'end' (which socketpair can't send).
 * Instead, it relies on the 'CONTENT_LENGTH' env var to know when to stop.
 * Your C++ server MUST set this environment variable for POSTs.
 *
 * @returns A promise that resolves with the complete body string.
 */
function readStdin(): Promise<string> {
	// 1. Get the exact length from the env var (passed by Webserv)
	const contentLengthStr = process.env.CONTENT_LENGTH || "0";
	const contentLength = parseInt(contentLengthStr, 10);

	// 2. Handle edge case: No body
	if (contentLength === 0) {
		return Promise.resolve("");
	}

	let data = "";
	let totalBytesRead = 0;

	return new Promise((resolve, reject) => {
		process.stdin.on("data", (chunk) => {
			data += chunk;
			totalBytesRead += chunk.length; // .length on a Buffer is bytes

			// 3. Manually check if we've read everything
			if (totalBytesRead >= contentLength) {
				// 4. We are done. Stop listening and resolve.
				process.stdin.pause(); // Stop the stream
				resolve(data); // Manually trigger the "end"
			}
		});

		process.stdin.on("error", (err) => reject(err));
	});
}

/**
 * The only function that prints to stdout.
 * Formats and sends the final CGI response.
 * @param status The CGI status line (e.g., "200 OK", "404 Not Found")
 * @param body The JavaScript object to send as a JSON response.
 */
function sendResponse(status: string, body: object) {
	console.log(`Status: ${status}`);
	console.log("Content-Type: application/json");
	console.log(""); // The critical blank line
	console.log(JSON.stringify(body, null, 2)); // Pretty-print JSON
}

/**
 * Handles GET requests (e.g., /user.ts?id=1)
 */
async function handleGet() {
	const params = new URLSearchParams(process.env.QUERY_STRING || "");
	const id = params.get("id");

	if (!id) {
		return sendResponse("400 Bad Request", {
			error: "Missing 'id' in query string.",
		});
	}

	const user = db.get(id);

	if (user) {
		return sendResponse("200 OK", user);
	} else {
		return sendResponse("404 Not Found", {
			error: `User with id ${id} not found.`,
		});
	}
}

/**
 * Handles POST requests (e.g., name=NewUser&title=Intern)
 */
async function handlePost() {
	// 1. Check Content-Type (as per your plan)
	const contentType = process.env.CONTENT_TYPE || "";
	if (contentType !== "application/x-www-form-urlencoded") {
		return sendResponse("415 Unsupported Media Type", {
			error: `Expected 'application/x-www-form-urlencoded', but got '${contentType}'`,
		});
	}

	// 2. Read the body from stdin (using our new hacked function)
	const bodyString = await readStdin();

	// 3. Parse the form data
	const params = new URLSearchParams(bodyString);
	const name = params.get("name");
	const title = params.get("title");

	// 4. Validate
	if (!name || !title) {
		return sendResponse("400 Bad Request", {
			error: "Missing 'name' or 'title' in form body.",
		});
	}

	// 5. Create new resource
	const newId = String(db.size + 1);
	const newUser = { id: newId, name, title };
	db.set(newId, newUser);

	// 6. Respond with 201 Created
	return sendResponse("201 Created", newUser);
}

/**
 * Handles any other method (PUT, DELETE, etc.)
 */
function handleMethodNotAllowed() {
	// The "Allow" header is crucial for a 405 response
	console.log("Allow: GET, POST");
	sendResponse("405 Method Not Allowed", {
		error: `Method ${process.env.REQUEST_METHOD} is not allowed.`,
	});
}

/**
 * The main entry point for the script.
 */
async function main() {
	try {
		switch (process.env.REQUEST_METHOD) {
			case "GET":
				await handleGet();
				break;
			case "POST":
				await handlePost();
				break;
			default:
				handleMethodNotAllowed();
		}
	} catch (error) {
		sendResponse("500 Internal Server Error", {
			error: "An unexpected error occurred.",
			detail: error instanceof Error ? error.message : "Unknown error",
		});
	}
}

// Run the script
main();
