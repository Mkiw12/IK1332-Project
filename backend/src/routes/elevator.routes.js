const express = require('express');
const elevatorController = require('../controllers/elevator.controller');

const router = express.Router();

// READ-ONLY endpoints (IoT device writes directly to Firestore)
router.get('/floor', elevatorController.getCurrentFloor);
router.get('/pattern', elevatorController.getTravelPattern);
router.get('/status', elevatorController.getStatus);
router.get('/history', elevatorController.getHistory);


module.exports = router;