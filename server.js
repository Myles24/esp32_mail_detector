const express = require('express');
const Speaker = require('speaker');
const bodyParser = require('body-parser');


const app = express();
const port = 3000;

// Root path route
app.get('/', (req, res) => {
  res.send('Hello, this is the root path!');
});

// Route to store WiFi signal strength
app.get('/storeSignalStrength', (req, res) => {
  const rssi = req.query.rssi;
  console.log(`Received WiFi Signal Strength: ${rssi} dBm`);
  playAlert(); // Play an audible alert
  res.sendStatus(200);
});

function playAlert() {
  const speaker = new Speaker({
        channels: 1,
        sampleRate: 22050,});

  const soundFile = 'mail4.wav'; // Play a sound
  const fs = require('fs');
  fs.createReadStream(soundFile).pipe(speaker);

//  speaker.close();
  setTimeout(() => {
    speaker.close();
 }, 2000); // Assuming the sound is 2 seconds long
}

app.listen(port, () => {
  console.log(`Server is running at http://localhost:${port}`);
});