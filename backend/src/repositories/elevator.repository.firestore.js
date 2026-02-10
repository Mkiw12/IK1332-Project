// src/repositories/elevator.repository.firestore.js
const db = require('../config/firestore');

class FirestoreElevatorRepository {
  constructor() {
    this.collection = db.collection('elevator-states');
  }

  /// Listen for updates on the latest document
  onStateChange(callback) {
    return this.collection
      .orderBy('timestamp', 'desc')
      .limit(1)
      .onSnapshot(snapshot => {
        if (!snapshot.empty) {
          const doc = snapshot.docs[0].data();
          callback(doc); // doc {currentFloor, pattern}
        }
      });
  }
}

module.exports = new FirestoreElevatorRepository();