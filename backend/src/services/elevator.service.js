const elevatorRepository  = require('../repositories/elevator.repository.firestore');

class ElevatorService {
    constructor() {

      this.latestState = {
      currentFloor: null,
      travelPattern: [],
      alarm: false,
      receivedAt: null
    };

    elevatorRepository.onStateChange(doc => {
      const currentFloor = doc.floor + 2;

      let travelPattern = this.latestState?.travelPattern || [];

      if (travelPattern[travelPattern.length - 1] !== currentFloor && currentFloor !== -1) {
        travelPattern.push(currentFloor);
      }

      if (travelPattern.length > 10) {
        travelPattern = travelPattern.slice(-10);
      }

      if (currentFloor === -1) {
        this.latestState.alarm = true;
      }

      const alarm = currentFloor === -1 || this.latestState.alarm;

      this.latestState = {
        currentFloor,
        travelPattern,
        receivedAt: doc.receivedAt,
        alarm
      };


      console.log('Updated state in service:', this.latestState);
    });
  }

  setState(state) {
    this.latestState = state;
  }

  async getDashboard() {
    if (!this.latestState) {
      return { currentFloor: null, pattern: [], alarm: null };
    }
    return {
      currentFloor: this.latestState.currentFloor,
      pattern: this.latestState.travelPattern || [],
      alarm: this.latestState.alarm
    };    
  }
}  

module.exports = new ElevatorService();