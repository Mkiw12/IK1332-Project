const elevatorService = require('../services/elevator.service');


getDashboard = async (req, res) => {
  try {
    const dashboard = await elevatorService.getDashboard();
    res.json(dashboard);
    console.log('Dashboard data sent response:', dashboard);
  } catch (err) {
    res.status(500).json({ error: 'Failed to fetch dashboard data' });
  }
};


module.exports = { getDashboard };