// src/repositories/elevator.repository.firestore.js
const db = require('../config/firestore');

class FirestoreElevatorRepository {
  constructor() {
    this.collection = db.collection('sensor_readings');
  }
  /// Listen for updates on the latest document
onStateChange(callback) {
  return this.collection
    .orderBy('receivedAt', 'desc')
    .limit(1)
    .onSnapshot(snapshot => {
      if (!snapshot.empty) {
        console.log('Received new snapshot from Firestore:', snapshot.docs[0].data());
        callback(snapshot.docs[0].data());
        }
      });
    }
}

module.exports = new FirestoreElevatorRepository();