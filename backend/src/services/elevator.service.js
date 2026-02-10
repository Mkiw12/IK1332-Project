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

  getCurrentFloor() {
    if (!this.latestState) throw new Error('No data yet');
    return this.latestState.currentFloor;
  }

  getPattern() {
    if (!this.latestState) throw new Error('No data yet');
    return this.latestState.pattern;
  }
}

module.exports = new ElevatorService();