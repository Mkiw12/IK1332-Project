const elevatorService = require('../services/elevator.service');

class ElevatorController {
  /**
   * GET /api/elevator/floor
   * Returns just the current floor number
   */
  async getCurrentFloor(req, res, next) {
    try {
      const floor = await elevatorService.getCurrentFloor();
      res.json({ currentFloor: floor });
    } catch (error) {
      next(error);
    }
  }

  /**
   * GET /api/elevator/pattern
   * Returns travel pattern with recent history
   */
  async getTravelPattern(req, res, next) {
    try {
      const pattern = await elevatorService.getTravelPattern();
      res.json(pattern);
    } catch (error) {
      next(error);
    }
  }

  /**
   * GET /api/elevator/status
   * Returns complete elevator status
   */
  async getStatus(req, res, next) {
    try {
      const status = await elevatorService.getStatus();
      res.json(status);
    } catch (error) {
      next(error);
    }
  }

  /**
   * GET /api/elevator/history?limit=20
   * Returns historical elevator data
   */
  async getHistory(req, res, next) {
    try {
      // Parse and validate limit parameter
      const limit = parseInt(req.query.limit) || 20;
      
      if (limit < 1 || limit > 100) {
        return res.status(400).json({
          error: 'Limit must be between 1 and 100'
        });
      }

      const history = await elevatorService.getHistory(limit);
      res.json(history);
    } catch (error) {
      next(error);
    }
  }
}

module.exports = new ElevatorController();