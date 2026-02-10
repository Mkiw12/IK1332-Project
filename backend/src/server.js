// Import the Express module
const express = require('express');
const path = require('path');
const elevatorRoutes = require('./routes/elevator.routes');
const elevatorRepository = require('./repositories/elevator.repository.firestore');
const elevatorService = require('./services/elevator.service');

const app = express();
const port = 3000;

app.use(express.json());

app.use(express.static(path.join(__dirname, '../public')));

// Mount the elevator routes at /elevator
app.use('/elevator', elevatorRoutes); 

elevatorRepository.onStateChange(state => {
  console.log('State updated:', state);
  elevatorService.setState(state);
});

// Start the server
app.listen(port, () => {
    console.log(`Server is running on http://localhost:${port}`);
});