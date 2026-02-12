#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "dsplp_io.h"

// --- SETTINGS ---
BMP581 pressureSensor;

const int WINDOW_SIZE = 20;     // Look at last 20 iterations
float historyBuffer[WINDOW_SIZE];
int bufferIndex = 0;
bool bufferFull = false;


//standard deviation start. estimated from .m 
const float JITTER_THRESHOLD = 2.5; 

void diodes(uint8_t leds) {
    for (int led = 0; led < 8; led++) {
        if (leds & (1 << led)) digitalWrite(LED_SDA_IO, HIGH);
        else digitalWrite(LED_SDA_IO, LOW);
        digitalWrite(LED_SHCP_IO, HIGH);
        delayMicroseconds(1);
        digitalWrite(LED_SHCP_IO, LOW);
    }
    digitalWrite(LED_STCP_IO, HIGH);
    delayMicroseconds(1);
    digitalWrite(LED_STCP_IO, LOW);
}

void setup() {
    Serial.begin(115200);
    Wire.begin(2, 1); // SDA, SCL

    pinMode(LED_SDA_IO, OUTPUT);
    pinMode(LED_SHCP_IO, OUTPUT);
    pinMode(LED_STCP_IO, OUTPUT);

    if (!pressureSensor.beginI2C()) {
        Serial.println("Sensor error.");
        while (1);
    }
    
    diodes(0); // Start dark
}

void loop() {
    bmp5_sensor_data data;
    
    if (pressureSensor.getSensorData(&data) == BMP5_OK) {
        historyBuffer[bufferIndex] = data.pressure;
        bufferIndex = (bufferIndex + 1) % WINDOW_SIZE;
        
        if (bufferIndex == 0) bufferFull = true;

        if (bufferFull) {
            float sum = 0;
            for (int i = 0; i < WINDOW_SIZE; i++) sum += historyBuffer[i];
            float mean = sum / WINDOW_SIZE;

            float varianceSum = 0;
            for (int i = 0; i < WINDOW_SIZE; i++) {
                varianceSum += pow(historyBuffer[i] - mean, 2);
            }
            float stdDev = sqrt(varianceSum / WINDOW_SIZE);

            // if jitter is high, we are moving.
            if (stdDev > JITTER_THRESHOLD) {
                Serial.printf("MOVING | Jitter: %.2f\n", stdDev);
                diodes(0xFF);
            } else {
                Serial.printf("STILL  | Jitter: %.2f\n", stdDev);
                diodes(0x00);
            }
        } else {
            Serial.println("Calibrating buffer...");
        }
    }
    
    delay(100);
}