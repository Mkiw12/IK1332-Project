#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "ICM_20948.h"
#include "../state_machine_classifier/dsplp_io.h"

// --- DSPLP PIN DEFINITIONS ---
#define SENSOR_SDA 2
#define SENSOR_SCL 1
#define WIRE_PORT Wire
#define AD0_VAL 1

const uint16_t SAMPLE_DELAY_MS = 100; // 10 Hz
const float MOTION_ACCEL_THRESHOLD_MG = 80.0f;

BMP581 pressureSensor;
ICM_20948_I2C myICM;

struct LogEntry {
    uint32_t timeMs;
    float pressurePa;
    float pressureDerivativePaPerSec;
    float accelMagMg;
    int imuMoving;
};
bool serialDebug = false;
LogEntry* flightData = NULL;
int totalSamples = 0;
bool isRecording = false;

float lastPressure = 0.0f;
uint32_t lastTimeMs = 0;
bool hasLastSample = false;

void dumpData();
void diodes(uint8_t leds);

void setup() {
    Serial.begin(115200);
    delay(1000);
    diodes(0);

    pinMode(SWA_IO, INPUT);
    pinMode(SWB_IO, INPUT);
    WIRE_PORT.begin(SENSOR_SDA, SENSOR_SCL);
    WIRE_PORT.setClock(400000);

    while (pressureSensor.beginI2C(BMP581_I2C_ADDRESS_DEFAULT) != BMP5_OK) {

        if (serialDebug) {
            Serial.println("Error: BMP581 not found!");
        }
        //error 1( everything but 0 in binary)
        //we show errors for when not serial available. in this case 0b11111110 is error 1
        diodes(0b11111110);
        
        delay(2000);
    }

    // SparkFun example style init. keep trying until sensor says ok.
    bool imuInitialized = false;
    while (!imuInitialized) {
        myICM.begin(WIRE_PORT, AD0_VAL);

        if (serialDebug) {
            Serial.print(F("Initialization of ICM-20948 returned: "));
            Serial.println(myICM.statusString());
        }

        if (myICM.status != ICM_20948_Stat_Ok) {
            if (serialDebug) {
                Serial.println("Trying again...");
            }
            delay(500);
        } else {
            imuInitialized = true;
        }
    }

    myICM.sleep(false);
    myICM.lowPower(false);
    myICM.setSampleMode((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Continuous);

    ICM_20948_fss_t myFSS;
    myFSS.a = gpm2;
    myFSS.g = dps250;
    myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);

    diodes(0);
}

void loop() {
    if (!isRecording) {
        if (digitalRead(SWA_IO) == HIGH) {
            isRecording = true;
            totalSamples = 0;
            hasLastSample = false;
            free(flightData);
            flightData = NULL;
            delay(200); // button debounce
        }
        return;
    }

    //gemini helping me with dynamic array in C
    LogEntry* resized = (LogEntry*)realloc(flightData, (totalSamples + 1) * sizeof(LogEntry));
    if (resized == NULL) {
        diodes(0xFF); // <-- dirty fail state if malloc blows up
        while (1) {}
    }
    flightData = resized;

    bmp5_sensor_data sensor;
    if (pressureSensor.getSensorData(&sensor) == BMP5_OK) {
        uint32_t nowMs = millis();
        float derivative = 0.0f;

        if (hasLastSample && nowMs > lastTimeMs) {
            float dt = (nowMs - lastTimeMs) / 1000.0f;
            derivative = (sensor.pressure - lastPressure) / dt;
        }

        float accelMagMg = 1000.0f;
        int imuMoving = 0;

        if (myICM.dataReady()) {
            myICM.getAGMT();

            float ax = myICM.accX();
            float ay = myICM.accY();
            float az = myICM.accZ();
            accelMagMg = sqrtf(ax * ax + ay * ay + az * az);

            // if total accel differs from 1g baseline, likely moving.
            imuMoving = (fabsf(accelMagMg - 1000.0f) > MOTION_ACCEL_THRESHOLD_MG) ? 1 : 0;
        }

        flightData[totalSamples].timeMs = nowMs;
        flightData[totalSamples].pressurePa = sensor.pressure;
        flightData[totalSamples].pressureDerivativePaPerSec = derivative;
        flightData[totalSamples].accelMagMg = accelMagMg;
        flightData[totalSamples].imuMoving = imuMoving;
        totalSamples++;

        lastPressure = sensor.pressure;
        lastTimeMs = nowMs;
        hasLastSample = true;
    }

    if (digitalRead(SWB_IO) == HIGH) {
        isRecording = false;
        dumpData();
    }

    delay(SAMPLE_DELAY_MS);
}

void dumpData() {
    Serial.println("\nTime_ms,Pressure_Pa,Pressure_dPaPerSec,AccelMag_mg,IMU_Moving");
    for (int i = 0; i < totalSamples; i++) {
        Serial.print(flightData[i].timeMs);
        Serial.print(",");
        Serial.print(flightData[i].pressurePa, 2);
        Serial.print(",");
        Serial.print(flightData[i].pressureDerivativePaPerSec, 3);
        Serial.print(",");
        Serial.print(flightData[i].accelMagMg, 2);
        Serial.print(",");
        Serial.println(flightData[i].imuMoving);
    }
    free(flightData);
    flightData = NULL;
    Serial.println("--- FINISHED ---");
    while (1);
}
// Function for controlling the LEDs via the shift register
// Each bit in 'leds' corresponds to one LED (0 = off, 1 = on).
void diodes(uint8_t leds) {
    // Shift out 8 bits, LSB first
    for (int led = 0; led < 8; led++) {
        // Prepare for a clock pulse on the shift clock
        digitalWrite(LED_SHCP_IO, LOW);

        // Write the current bit to the data line
        // (1 << led) creates a mask to isolate one bit at a time
        if (leds & (1 << led)) {
            digitalWrite(LED_SDA_IO, HIGH);
        } else {
            digitalWrite(LED_SDA_IO, LOW);
        }

        delayMicroseconds(1);    // Small setup time before clock

        // Rising edge on the shift clock: the bit is shifted into the register
        digitalWrite(LED_SHCP_IO, HIGH);
        delayMicroseconds(1);    // Small hold time after clock
    }

    // Optional: set lines low after shifting
    digitalWrite(LED_SHCP_IO, LOW);
    digitalWrite(LED_SDA_IO, LOW);

    // Latch the shifted data into the output register:
    // Rising edge on STCP transfers the internal register to the outputs
    digitalWrite(LED_STCP_IO, HIGH);
    delayMicroseconds(1);
    digitalWrite(LED_STCP_IO, LOW);
}

