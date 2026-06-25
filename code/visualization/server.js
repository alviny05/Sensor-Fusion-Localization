const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const WebSocket = require('ws');
const express = require('express');

const app = express();
const server = app.listen(8080, () => {
  console.log("Server running at http://localhost:8080");
});

app.use(express.static("public"));

const wss = new WebSocket.Server({ server });

// serial
const port = new SerialPort({
  path: 'COM8',
  baudRate: 115200
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

parser.on('data', (line) => {
  const values = line.trim().split(',');
  if (values.length !== 6) return;

  const [xa, ya, za, xd, yd, zd] = values.map(Number);
  console.log(values);

  const payload = JSON.stringify({ xa, ya, za, xd, yd, zd});

  // broadcast to browser
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(payload);
    }
  });
});