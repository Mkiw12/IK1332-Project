/**
 * Import function triggers from their respective submodules:
 *
 * const {onCall} = require("firebase-functions/v2/https");
 * const {onDocumentWritten} = require("firebase-functions/v2/firestore");
 *
 * See a full list of supported triggers at https://firebase.google.com/docs/functions
 */

const {setGlobalOptions} = require("firebase-functions");
const {onRequest} = require("firebase-functions/https");
const logger = require("firebase-functions/logger");
const { onDocumentUpdated } = require("firebase-functions/v2/firestore");


const {initializeApp} = require("firebase-admin/app");
const {getFirestore} = require("firebase-admin/firestore");
const nodemailer = require("nodemailer");
var config = require("./config");

initializeApp();

// For cost control, you can set the maximum number of containers that can be
// running at the same time. This helps mitigate the impact of unexpected
// traffic spikes by instead downgrading performance. This limit is a
// per-function limit. You can override the limit for each function using the
// `maxInstances` option in the function's options, e.g.
// `onRequest({ maxInstances: 5 }, (req, res) => { ... })`.
// NOTE: setGlobalOptions does not apply to functions using the v1 API. V1
// functions should each use functions.runWith({ maxInstances: 10 }) instead.
// In the v1 API, each function can only serve one request per container, so
// this will be the maximum concurrent request count.
setGlobalOptions({ maxInstances: 1 });

// Create and deploy your first functions
// https://firebase.google.com/docs/functions/get-started

// exports.helloWorld = onRequest((request, response) => {
//   logger.info("Hello logs!", {structuredData: true});
//   response.send("Hello from Firebase!");
// });

exports.onFloorStuck = onDocumentUpdated(
  "sensor_readings/{docId}", // docId is the timestamp document name
  async (event) => {
    const before = event.data.before.data();
    const after = event.data.after.data();

    if (!before || !after) return;

    const prevFloor = before.floor;
    const newFloor = after.floor;

    // Only trigger when floor changes to -1
    if (prevFloor !== -1 && newFloor === -1) {
      logger.info("betweenFloors changed to true. Sending email...");

      // Send email here
      const transporter = nodemailer.createTransport({
        service: "gmail",
        auth: {
          user: config.fromMail,  
          pass: config.password // use App Password if 2FA enabled
        },
      });

       const mailOptions = {
        from: '"Elevator Alert" <' + config.fromMail + '>',
        to: config.toMail,
        subject: "Elevator Alert: Between Floors",
        text: `The elevator is now between floors.`,
      }

      try {
        await transporter.sendMail(mailOptions);
        console.log("Email sent successfully!");
      } catch (err) {
        console.error("Error sending email:", err);
      } 
    }
  }
);
