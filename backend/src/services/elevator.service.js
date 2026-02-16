const elevatorRepository  = require('../repositories/elevator.repository.firestore');

class ElevatorService {
    constructor() {
        this.latestState = null;

    elevatorRepository.onStateChange(state => {
      this.latestState = state;
      console.log('Updated state in service:', state);
    });
  }

  setState(state) {
    this.latestState = state;
  }

  async getDashboard() {
    if (!this.latestState) {
      return { currentFloor: null, pattern: [], alarms: null };
    }
    return {
      currentFloor: this.latestState.currentFloor,
      pattern: this.latestState.pattern || [],
      alarms: this.latestState.alarms
    };    
  }
}  

module.exports = new ElevatorService();