const admin = require('firebase-admin');
const path = require('path');
const serviceAccount = require(path.join(__dirname, 'firebase-key.json'));

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://ik1332proj-default-rtdb.europe-west1.firebasedatabase.app"
});

const db = admin.firestore();

module.exports = db;
