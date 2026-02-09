//const admin = require('firebase-admin');

// TODO: Initialize Firestore
// Option 1: Use service account JSON file
// const serviceAccount = require('./path/to/serviceAccountKey.json');
// admin.initializeApp({
//   credential: admin.credential.cert(serviceAccount)
// });

// Option 2: Use application default credentials (for deployed environments)
// admin.initializeApp({
//   credential: admin.credential.applicationDefault()
// });

// For now, return null until configured
let db = null;

try {
  // Uncomment when ready to connect
  // admin.initializeApp({
  //   credential: admin.credential.applicationDefault()
  // });
  // db = admin.firestore();
} catch (error) {
  console.warn('Firestore not initialized. Using mock data.');
}

//module.exports = { db, admin };