// Import the Express module
const express = require('express');
const path = require('path');
const elevatorRoutes = require('./routes/elevator.routes');
const app = express();
const port = 3000;

app.use(express.json());

app.use(express.static(path.join(__dirname, '../public')));

// Mount the elevator routes at /elevator
app.use('/elevator', elevatorRoutes); 


// Start the server
app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
});