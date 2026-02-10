const elevatorService = require('../services/elevator.service');


  function getCurrentFloor(req, res) {
  try {
    const floor = elevatorService.getCurrentFloor();
    res.json({ currentFloor: floor });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
}

function getPattern(req, res) {
  try {
    const pattern = elevatorService.getPattern();
    res.json({ pattern });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
}


module.exports = { getCurrentFloor, getPattern };