#!/usr/bin/env ts-node-script --swc
"use strict";
/**
 * A CGI script (using ts-node) for a simple User API.
 * - GET: Responds to /user.ts?id=1 (returns JSON)
 * - GET: Responds to /user.ts (returns all users)
 * - POST: Responds to form data (application/x-www-form-urlencoded)
 * - OUTPUT: Always returns JSON.
 */
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g = Object.create((typeof Iterator === "function" ? Iterator : Object).prototype);
    return g.next = verb(0), g["throw"] = verb(1), g["return"] = verb(2), typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (g && (g = 0, op[0] && (_ = 0)), _) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
Object.defineProperty(exports, "__esModule", { value: true });
var fs = require("fs");
var path = require("path");
// Path to persistent storage
var DB_FILE = path.join(__dirname, "users.json");
// --- Persistent Database ---
function loadDB() {
    try {
        if (fs.existsSync(DB_FILE)) {
            var data = fs.readFileSync(DB_FILE, "utf-8");
            var obj = JSON.parse(data);
            return new Map(Object.entries(obj));
        }
    }
    catch (err) {
        // If file doesn't exist or is corrupt, start fresh
    }
    // Initialize with default users
    var db = new Map();
    db.set("1", { id: "1", name: "Gigachad", title: "CEO" });
    db.set("2", { id: "2", name: "Colin", title: "Dev" });
    saveDB(db);
    return db;
}
function saveDB(db) {
    var obj = Object.fromEntries(db);
    fs.writeFileSync(DB_FILE, JSON.stringify(obj, null, 2));
}
var db = loadDB();
/**
 * Asynchronously reads the full request body from stdin.
 *
 * HACK: This version does NOT wait for 'end' (which socketpair can't send).
 * Instead, it relies on the 'CONTENT_LENGTH' env var to know when to stop.
 * Your C++ server MUST set this environment variable for POSTs.
 *
 * @returns A promise that resolves with the complete body string.
 */
function readStdin() {
    // 1. Get the exact length from the env var (passed by Webserv)
    var contentLengthStr = process.env.CONTENT_LENGTH || "0";
    var contentLength = parseInt(contentLengthStr, 10);
    // 2. Handle edge case: No body
    if (contentLength === 0) {
        return Promise.resolve("");
    }
    var data = "";
    var totalBytesRead = 0;
    return new Promise(function (resolve, reject) {
        process.stdin.on("data", function (chunk) {
            data += chunk;
            totalBytesRead += chunk.length; // .length on a Buffer is bytes
            // 3. Manually check if we've read everything
            if (totalBytesRead >= contentLength) {
                // 4. We are done. Stop listening and resolve.
                process.stdin.pause(); // Stop the stream
                resolve(data); // Manually trigger the "end"
            }
        });
        process.stdin.on("error", function (err) { return reject(err); });
    });
}
/**
 * The only function that prints to stdout.
 * Formats and sends the final CGI response.
 * @param status The CGI status line (e.g., "200 OK", "404 Not Found")
 * @param body The JavaScript object to send as a JSON response.
 */
function sendResponse(status, body) {
    console.log("Status: ".concat(status));
    console.log("Content-Type: application/json");
    console.log(""); // The critical blank line
    console.log(JSON.stringify(body, null, 2)); // Pretty-print JSON
}
/**
 * Handles GET requests
 * - /user.ts?id=1 (returns single user)
 * - /user.ts (returns all users)
 */
function handleGet() {
    return __awaiter(this, void 0, void 0, function () {
        var params, id, allUsers, user;
        return __generator(this, function (_a) {
            params = new URLSearchParams(process.env.QUERY_STRING || "");
            id = params.get("id");
            // If no ID provided, return ALL users
            if (!id) {
                allUsers = Array.from(db.values());
                return [2 /*return*/, sendResponse("200 OK", { users: allUsers })];
            }
            user = db.get(id);
            if (user) {
                return [2 /*return*/, sendResponse("200 OK", user)];
            }
            else {
                return [2 /*return*/, sendResponse("404 Not Found", {
                        error: "User with id ".concat(id, " not found."),
                    })];
            }
            return [2 /*return*/];
        });
    });
}
/**
 * Handles POST requests (e.g., name=NewUser&title=Intern)
 */
function handlePost() {
    return __awaiter(this, void 0, void 0, function () {
        var contentType, bodyString, params, name, title, newId, newUser;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    contentType = process.env.CONTENT_TYPE || "";
                    if (contentType !== "application/x-www-form-urlencoded") {
                        return [2 /*return*/, sendResponse("415 Unsupported Media Type", {
                                error: "Expected 'application/x-www-form-urlencoded', but got '".concat(contentType, "'"),
                            })];
                    }
                    return [4 /*yield*/, readStdin()];
                case 1:
                    bodyString = _a.sent();
                    params = new URLSearchParams(bodyString);
                    name = params.get("name");
                    title = params.get("title");
                    // 4. Validate
                    if (!name || !title) {
                        return [2 /*return*/, sendResponse("400 Bad Request", {
                                error: "Missing 'name' or 'title' in form body.",
                            })];
                    }
                    newId = String(db.size + 1);
                    newUser = { id: newId, name: name, title: title };
                    db.set(newId, newUser);
                    saveDB(db); // Persist to file
                    // 6. Respond with 201 Created
                    return [2 /*return*/, sendResponse("201 Created", newUser)];
            }
        });
    });
}
/**
 * Handles any other method (PUT, DELETE, etc.)
 */
function handleMethodNotAllowed() {
    // The "Allow" header is crucial for a 405 response
    console.log("Allow: GET, POST");
    sendResponse("405 Method Not Allowed", {
        error: "Method ".concat(process.env.REQUEST_METHOD, " is not allowed."),
    });
}
/**
 * The main entry point for the script.
 */
function main() {
    return __awaiter(this, void 0, void 0, function () {
        var _a, error_1;
        return __generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    _b.trys.push([0, 7, , 8]);
                    _a = process.env.REQUEST_METHOD;
                    switch (_a) {
                        case "GET": return [3 /*break*/, 1];
                        case "POST": return [3 /*break*/, 3];
                    }
                    return [3 /*break*/, 5];
                case 1: return [4 /*yield*/, handleGet()];
                case 2:
                    _b.sent();
                    return [3 /*break*/, 6];
                case 3: return [4 /*yield*/, handlePost()];
                case 4:
                    _b.sent();
                    return [3 /*break*/, 6];
                case 5:
                    handleMethodNotAllowed();
                    _b.label = 6;
                case 6: return [3 /*break*/, 8];
                case 7:
                    error_1 = _b.sent();
                    sendResponse("500 Internal Server Error", {
                        error: "An unexpected error occurred.",
                        detail: error_1 instanceof Error ? error_1.message : "Unknown error",
                    });
                    return [3 /*break*/, 8];
                case 8: return [2 /*return*/];
            }
        });
    });
}
// Run the script
main();
