// app.js
const express = require('express');
const { spawn } = require('child_process');
const bodyParser = require('body-parser');

const cors = require('cors');

const app = express();
const port = 3000;

// Middleware to enable CORS
app.use(cors());

// Middleware to parse JSON
app.use(bodyParser.json());

app.post('/api/simulate', (req, res) => {
    const requestData = req.body;
    const inputData = JSON.stringify(requestData);

    const chunks = [];

    const cAppProcess = spawn('./Business Logic/bin/main');
    cAppProcess.stdin.write(inputData);
    cAppProcess.stdin.end();


    // Listen for output from the C application
    cAppProcess.stdout.on('data', (data) => {
        chunks.push(data);
    });

    cAppProcess.stderr.on('data', (data) => {
        console.error(`stderr: ${data}`);
    });

    // Handle process exit
    cAppProcess.on('close', (code) => {
        if (code === 0) {
            // Concatenate the chunks to get the complete output
            const output = Buffer.concat(chunks).toString('utf-8');
            json = JSON.parse(output);
            console.log(`child process exited with code ${code}`);
            res.status(200).json(json);
        } else {
            console.error(`child process exited with code ${code}`);
            res.status(500).json({ error: 'Internal Server Error' });
        }
    });
});


// Start the server
app.listen(port, () => {
    console.log(`Server is listening at http://localhost:${port}`);
});
