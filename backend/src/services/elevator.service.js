const elevatorRepository  = require('../repositories/elevator.repository.firestore');
const Elevator = require('../model/elevator.model');

class ElevatorService {
  /**
   * Get just the current floor number
   * @returns {Promise<number>}
   */
  async getCurrentFloor() {
    const data = await elevatorRepository.getCurrentState();
    if (!data) {
      throw new Error('No elevator data available');
    }
    const elevator = new Elevator(data);
    return elevator.currentFloor;
  }

  /**
   * Get travel pattern with recent movement history
   * @returns {Promise<Object>}
   */
  async getTravelPattern() {
    const historyData = await elevatorRepository.getTravelHistory(10);
    
    if (historyData.length === 0) {
      throw new Error('No elevator data available');
    }

    const history = historyData.map(data => new Elevator(data));
    const current = history[history.length - 1];

    return {
      currentFloor: current.currentFloor,
      direction: current.direction,
      pattern: current.getTravelPattern(),
      confidence: current.confidence,
      recentStops: history.map(h => ({
        floor: h.currentFloor,
        timestamp: h.timestamp,
        direction: h.direction,
        pressure: h.pressure
      }))
    };
  }

  /**
   * Get complete elevator status
   * @returns {Promise<Object>}
   */
  async getStatus() {
    const data = await elevatorRepository.getCurrentState();
    
    if (!data) {
      throw new Error('No elevator data available');
    }

    const elevator = new Elevator(data);

    return {
      currentFloor: elevator.currentFloor,
      direction: elevator.direction,
      status: elevator.status,
      travelPattern: elevator.getTravelPattern(),
      pressure: elevator.pressure,
      pressureChange: elevator.pressureChange,
      confidence: elevator.confidence,
      isMoving: elevator.isMoving(),
      lastUpdate: elevator.timestamp
    };
  }

  /**
   * Get historical elevator data
   * @param {number} limit - Number of records to retrieve
   * @returns {Promise<Array>}
   */
  async getHistory(limit = 20) {
    const historyData = await elevatorRepository.getTravelHistory(limit);
    return historyData.map(data => new Elevator(data));
  }
}

module.exports = new ElevatorService();