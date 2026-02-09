class Elevator {
  constructor(data) {
    this.id = data.id;
    this.currentFloor = data.currentFloor;
    this.direction = data.direction; // 'up', 'down', 'idle'
    this.status = data.status; // 'operational', 'maintenance', 'error'
    this.pressure = data.pressure; // Current pressure reading in Pa
    this.pressureChange = data.pressureChange || 0; // Pa/s
    this.confidence = data.confidence || 'high'; // 'high', 'medium', 'low'
    this.timestamp = data.timestamp;
  }

  getTravelPattern() {
    if (this.direction === 'idle') {
      return `Stationary at floor ${this.currentFloor}`;
    }
    return `Moving ${this.direction} from floor ${this.currentFloor}`;
  }

  isMoving() {
    return this.direction !== 'idle';
  }
}

module.exports = Elevator;