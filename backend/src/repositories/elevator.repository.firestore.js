const { db } = require('../config/firestore');
const state = {
  currentFloor: 1,
  direction: "UP",
  motionState: "IDLE",
  history: [1]
};


class FirestoreElevatorRepository {
  
  constructor() {
    this.elevatorId = 'elevator-1';
    // Points to where IoT device writes data
    this.collection = null; // Will be: db.collection('elevator-states')
  }
  

  // READ ONLY - Get latest state that IoT device wrote
  async getCurrentState() {
    // TODO: Query Firestore for most recent document
    // IoT device has already written this data
    // Example:
    // const snapshot = await this.collection
    //   .orderBy('timestamp', 'desc')
    //   .limit(1)
    //   .get();
    // 
    // if (snapshot.empty) return null;
    // const doc = snapshot.docs[0];
    // return { id: this.elevatorId, ...doc.data() };

    
    return state; // Mock data until Firestore is set up
    
    throw new Error('Firestore not configured. Using mock repository.');
  }

  // READ ONLY - Get historical states that IoT device wrote
  async getTravelHistory(limit = 20) {
    // TODO: Query Firestore for recent documents
    // Example:
    // const snapshot = await this.collection
    //   .orderBy('timestamp', 'desc')
    //   .limit(limit)
    //   .get();
    // 
    // return snapshot.docs
    //   .map(doc => ({ id: this.elevatorId, ...doc.data() }))
    //   .reverse();
    
    throw new Error('Firestore not configured. Using mock repository.');
  }

  // NO addReading() method - IoT device writes directly
}

module.exports = new FirestoreElevatorRepository();