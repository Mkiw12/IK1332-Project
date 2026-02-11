#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "dsplp_io.h"

// --- DSPLP PIN DEFINITIONS ---
#define SENSOR_SDA 2
#define SENSOR_SCL 1

BMP581 pressureSensor;

struct LogEntry {
    uint32_t time;
    float pressure;
    int floorMarker;
};

LogEntry* flightData = NULL;
int totalSamples = 0;
bool isRecording = false;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(SWA_IO, INPUT);
    pinMode(SWB_IO, INPUT);
    Wire.begin(SENSOR_SDA, SENSOR_SCL);

    while(pressureSensor.beginI2C(BMP581_I2C_ADDRESS_DEFAULT) != BMP5_OK) {
        Serial.println("Error: BMP581 not found!");
        delay(2000);
    }

    Serial.println("Ready. HOLD A to signify Floor 0 and start recording.");
    
    while(digitalRead(SWA_IO) == LOW);
    isRecording = true;
}

void loop() {
    if (isRecording) {
        //gemini helping me with dynamic array in C
        flightData = (LogEntry*)realloc(flightData, (totalSamples + 1) * sizeof(LogEntry));

        bmp5_sensor_data sensor;
        if (pressureSensor.getSensorData(&sensor) == BMP5_OK) {
            flightData[totalSamples].time = millis();
            flightData[totalSamples].pressure = sensor.pressure;
            // Marker is 1 if A is held, 0 if moving
            flightData[totalSamples].floorMarker = (digitalRead(SWA_IO) == HIGH) ? 1 : 0;
            totalSamples++;
        }

        if (digitalRead(SWB_IO) == HIGH) {
            isRecording = false;
            dumpData();
        }

        delay(50); // 20hz sampling
    }
}

void dumpData() {
    Serial.println("\nTime_ms, Pressure_Pa, Floor_Marker");
    for (int i = 0; i < totalSamples; i++) {
        Serial.print(flightData[i].time);
        Serial.print(", ");
        Serial.print(flightData[i].pressure, 2);
        Serial.print(", ");
        Serial.println(flightData[i].floorMarker);
    }
    free(flightData);
    Serial.println("--- FINISHED ---");
    while(1);
}