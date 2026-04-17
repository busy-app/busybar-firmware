import { WebSocketServer } from "ws";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

// Define variables
const USE_SSL = false; // Set to true to use WSS (WebSocket Secure), false to use WS (WebSocket)
const HOSTNAME = "example.com";
const PATHNAME = "/myresource";
const PORT = 8080;

// Conditionally import the HTTP or HTTPS module
const { createServer } = USE_SSL ? await import("https") : await import("http");

// Get the directory name
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Load SSL certificates if USE_SSL is true
const options = USE_SSL ? {
    key: fs.readFileSync(path.resolve(__dirname, "../certificates/server-key.pem")),
    cert: fs.readFileSync(path.resolve(__dirname, "../certificates/server-cert.pem")),
} : {};

// Create an HTTP or HTTPS server based on USE_SSL
const server = USE_SSL ? createServer(options, (req, res) => {
    res.writeHead(404);
    res.end();
}) : createServer((req, res) => {
    res.writeHead(404);
    res.end();
});

// Create a WebSocket server
const wss = new WebSocketServer({ noServer: true, maxPayload: 100 * 1024 * 1024 });

// Handle WebSocket connections
wss.on("connection", function connection(ws, req) {
    // Print the IP address of the client
    let clientIp = req.socket.remoteAddress;
    if (clientIp.startsWith("::ffff:")) {
        clientIp = clientIp.replace("::ffff:", "");
    }
    console.log(`Client connected: ${clientIp}`);


    ws.on("ping", (data) => {
        console.log("Received ping from client with payload:", data.toString());
    });

    ws.on("pong", (data) => {
        console.log("Received pong from client with payload:", data.toString());
    });


    ws.on("message", function message(data) {
        try {
            console.log("received: %s", data);
        } catch (error) {
            console.error("Error processing message:", error);
            ws.close(1002, "Invalid message format");
        }
    });

    ws.on("error", function error(err) {
        console.error("WebSocket error:", err);
    });

    ws.on("close", function close(code, reason) {
        console.log(`WebSocket closed: ${code} - ${reason}`);
    });

    let messageCount = 0;
    const maxMessages = 5;

    const intervalId = setInterval(() => {
        try {
            if (ws.readyState === ws.OPEN) {
                if (messageCount < maxMessages) {
                    const message = "Hello, World!";
                    ws.send(message);
                    messageCount++;
                } else {
                    clearInterval(intervalId);
                }
            } else {
                clearInterval(intervalId);
            }
        } catch (error) {
            console.error("Error sending data:", error);
            clearInterval(intervalId);
        }
    }, 1000);

    // Send ping packet every 10 seconds
    const pingInterval = setInterval(() => {
        if (ws.readyState === ws.OPEN) {
            ws.ping();
        }
    }, 10000);

});

// Handle HTTP upgrade requests
server.on("upgrade", (req, socket, head) => {
    const protocol = USE_SSL ? "https" : "http";
    const { pathname, hostname } = new URL(req.url, `${protocol}://${req.headers.host}`);
    console.log(`Upgrade request for hostname: ${hostname}, pathname: ${pathname}`);

    if ((hostname === HOSTNAME && pathname === PATHNAME) || hostname === "localhost") {
        wss.handleUpgrade(req, socket, head, (ws) => {
            wss.emit("connection", ws, req);
        });
    } else {
        console.log("Invalid hostname or pathname, destroying socket");
        socket.destroy();
    }
});

// Start the server
server.listen(PORT, () => {
    console.log(`Server is listening on port ${PORT}`);
});
