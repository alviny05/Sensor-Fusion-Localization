const fs = require('fs');
const WebSocket = require('ws');
const express = require('express');

const app = express();
const server = app.listen(8080, () => {
  console.log("Server running at http://localhost:8080");
  console.log("Streaming CSV data to browser...");
});

app.use(express.static("public"));

const wss = new WebSocket.Server({ server });

const CSV_FILE = 'data.csv'; 
//const CSV_FILE = 'phoneData.csv'; 

const INTERVAL = 100;        // 0.1 seconds (100ms)

let dataBuffer = [];
let currentIndex = 0;

function loadCSV() {
  try {
    const fileContent = fs.readFileSync(CSV_FILE, 'utf8');
    // Split by lines and remove empty lines or headers
    const lines = fileContent.split(/\r?\n/).filter(line => line.trim() !== "");

    dataBuffer = lines;
    console.log(`Loaded ${dataBuffer.length} rows from ${CSV_FILE}`);
  } catch (err) {
    console.error("Error reading CSV file:", err.message);
    process.exit(1);
  }
}

loadCSV();

setInterval(() => {
  if (dataBuffer.length === 0) return;

  // Get current line and split it
  const line = dataBuffer[currentIndex];
  const values = line.split(',').map(Number);

  // Validate 6 columns
  if (values.length === 6) {
    const [xa, ya, za, xd, yd, zd] = values;
    
    const payload = JSON.stringify({ xa, ya, za, xd, yd, zd });

    // Broadcast to browser
    wss.clients.forEach(client => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(payload);
      }
    });
  }

  // Move to next line
  currentIndex = (currentIndex + 1) % dataBuffer.length;
}, INTERVAL);