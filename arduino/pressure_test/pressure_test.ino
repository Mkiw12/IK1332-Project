#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "dsplp_io.h"

#define SENSOR_SDA 2
#define SENSOR_SCL 1

BMP581 pressureSensor;
int initialDataPoints = 30;

// Dynamic memory pointers
float** dataStorage = NULL; 
int totalGroups = 0;

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

    Serial.println("Ready. Press A to record a floor. Press B to dump CSV-ish.");
}

void loop() {
    if (digitalRead(SWA_IO) == HIGH) {
        Serial.print("Recording Group ");
        Serial.println(totalGroups);

        //gemini helping me with dynamic array in C
        // 1. Expand the "Rows" of our 2D array
        dataStorage = (float**)realloc(dataStorage, (totalGroups + 1) * sizeof(float*));
        // 2. Allocate the "Columns" (the 30 samples)
        dataStorage[totalGroups] = (float*)malloc(initialDataPoints * sizeof(float));

        //collect data
        for (int i = 0; i < initialDataPoints; i++) {
            dataStorage[totalGroups][i] = getPa();
            delay(500);
        }

        Serial.println("Floor recorded.");
        totalGroups++;
        
        while(digitalRead(SWA_IO) == HIGH);
    }
    if (digitalRead(SWB_IO) == HIGH) {
        Serial.println("\nGroup_ID, Sample_N, Pascal");
        for (int g = 0; g < totalGroups; g++) {
            for (int n = 0; n < initialDataPoints; n++) {
                Serial.print(g);
                Serial.print(", ");
                Serial.print(n);
                Serial.print(", ");
                Serial.println(dataStorage[g][n]);
            }
            // Free memory as we go to keep the board clean
            free(dataStorage[g]);
        }
        free(dataStorage);
        Serial.println("--- FINISHED ---");
        while(1);
    }
}

float getPa() {
    bmp5_sensor_data data = {0,0};
    if (pressureSensor.getSensorData(&data) == BMP5_OK) {
        return data.pressure;
    }
    return -1.0;
}