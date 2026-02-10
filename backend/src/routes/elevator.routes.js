const express = require('express');
const elevatorController = require('../controllers/elevator.controller');
const router = express.Router();

// READ-ONLY endpoints (IoT device writes directly to Firestore)
router.get('/floor', elevatorController.getCurrentFloor);
router.get('/pattern', elevatorController.getPattern);;


module.exports = router;