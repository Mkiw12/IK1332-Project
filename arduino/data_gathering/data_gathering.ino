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
bool serialDebug = false;
LogEntry* flightData = NULL;
int totalSamples = 0;
bool isRecording = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    diodes(0);

    pinMode(SWA_IO, INPUT);
    pinMode(SWB_IO, INPUT);
    Wire.begin(SENSOR_SDA, SENSOR_SCL);

    while(pressureSensor.beginI2C(BMP581_I2C_ADDRESS_DEFAULT) != BMP5_OK) {

        if(serialDebug){
            Serial.println("Error: BMP581 not found!");
        }
        //error 1( everything but 0 in binary)
        //we show errors for when not serial available. in this case 0b11111110 is error 1
        diodes(0b11111110)
        
        delay(2000);
    }
    else{
        diodes(0);
    }
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
        delay(100); // 10hz sampling
    }
    derivative = ???
    //calculate derivative

    if(derivative > theta + 0 || derivative < -theta + 0){
        //detected stop
    }
    else{}
    //if moving code try and figure out how many floors diff is since last calculation? 

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

