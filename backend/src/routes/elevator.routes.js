const express = require('express');
const elevatorController = require('../controllers/elevator.controller');
const router = express.Router();

// READ-ONLY endpoints (IoT device writes directly to Firestore)
router.get('/dashboard', elevatorController.getDashboard);
//router.put('/alarms', elevatorController.setAlarms);


module.exports = router;