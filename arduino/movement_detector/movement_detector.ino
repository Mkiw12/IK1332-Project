#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "dsplp_io.h"
#include "WiFi.h"

// --- SETTINGS ---
BMP581 pressureSensor;
uint8_t i2cAddress = BMP581_I2C_ADDRESS_DEFAULT; 

const int WINDOW_SIZE = 20;     
float historyBuffer[WINDOW_SIZE];
int bufferIndex = 0;
bool bufferFull = false;

// offsets calculated at matlab or online learning? needs to be checked
float FLOOR_OFFSETS[] = {0,  -39.6733,  -75.8909, -113.5566, -151.4735, -185.2196}; 
const int NUM_FLOORS = sizeof(FLOOR_OFFSETS) / sizeof(FLOOR_OFFSETS[0]);
const float FLOOR_THRESHOLD = 10.0; 

float referencePressure = 0; 
const float JITTER_THRESHOLD = 1.5;

// --- FUNCTION PROTOTYPES ---
void handleMovingState(float stdDev);
void handleStillState(float meanPressure);
void diodes(uint8_t leds);

void setup() {
    Serial.begin(115200);
    Wire.begin(2, 1); // SDA, SCL

    pinMode(LED_SDA_IO, OUTPUT);
    pinMode(LED_SHCP_IO, OUTPUT);
    pinMode(LED_STCP_IO, OUTPUT);

    if (pressureSensor.beginI2C(i2cAddress) != BMP5_OK) {
        Serial.println("Sensor error.");
        while (1);
    }
    
    // Initial Calibration (Assume starting at Floor 0)
    //TODO: Replace with 2nd sensor? 
    bmp5_sensor_data data;

    Serial.println("Stabilizing for 2 seconds...");
    delay(2000); 

    float sum = 0;
    int samples = 100; // More samples = better average
    for(int i = 0; i < samples; i++) {
        pressureSensor.getSensorData(&data);
        sum += data.pressure;
        delay(10);
    }
    referencePressure = sum / (float)samples;
    Serial.printf("Reference Pressure: %.2f\n", referencePressure);

    diodes(0); 
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
            if (stdDev > JITTER_THRESHOLD) {
                handleMovingState(stdDev);
            } else {
                handleStillState(mean);
            }
        }
    } else {
        Serial.println("SENSOR ERROR");
    }
    
    delay(100);
}

/**
 * Handles behavior when the elevator is moving.
 * Currently keeps all LEDs ON to show active movement.
 */
void handleMovingState(float stdDev) {
    Serial.printf("MOVING | Jitter: %.2f\n", stdDev);
    diodes(0xFF); 
}

/**
 * Handles behavior when the elevator is stationary.
 * Checks the pressure against your MATLAB-derived floor map. Kind of a mess a bit
 */
void handleStillState(float meanPressure) {
    float currentDelta = meanPressure - referencePressure;
    int detectedFloor = -1;
    float learningRate = 0.05; // how much to adjust after found floor

    for (int i = 0; i < NUM_FLOORS; i++) {
        if (abs(currentDelta - FLOOR_OFFSETS[i]) < FLOOR_THRESHOLD) {
            detectedFloor = i;
            FLOOR_OFFSETS[i] = (FLOOR_OFFSETS[i] * (1.0 - learningRate)) + (currentDelta * learningRate); //same as learning
            break;
        }
    }

    if (detectedFloor != -1) {
        // still but floor exists (when not unknown (-1))
        Serial.printf("STILL | Floor: %d | Delta: %.2f | New Offset: %.2f\n", 
                      detectedFloor, currentDelta, FLOOR_OFFSETS[detectedFloor]);
        
        diodes( ~(1 << detectedFloor) ); 
    } else {
        // could be the elevator is stuck between floors or the map is significantly off.
        Serial.printf("STILL | Unknown | Delta: %.2f | meanPressure %.2f\n", currentDelta, meanPressure);
        diodes(0x00); 
    }
}

/**
 * Low-level shift register control for the LEDs.
 */
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