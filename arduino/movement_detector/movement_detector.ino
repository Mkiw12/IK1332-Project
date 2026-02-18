#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "dsplp_io.h"
#include "WiFi.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
// CA certificate
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";
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


//-- WIFI SETTINGS
const char *ssid = "Jacob";
const char *password = "jacob12345";
//wifi/mqtt prototypes 
void connectWifi();
void connectMQTT();
void callback(char *topic, byte *payload, unsigned int length);
void publishTelemetry(const char* state, int floor, float mean, float stdev);
// MQTT Broker
const char *mqtt_broker = "jf9611d6.ala.eu-central-1.emqxsl.com";
const char *topic = "ik1332/proj/sensors";
const char *mqtt_username = "test";
const char *mqtt_password = "test";
const int mqtt_port = 8883;


WiFiClientSecure espClient;
PubSubClient client(espClient);


void setup() {
    Serial.begin(115200);
    Wire.begin(2, 1); // SDA, SCL


    delay(2000);
//wifi settings
// mac address edvin board
// 30:ED:A0:DC:E3:D0
  espClient.setCACert(ca_cert);

  connectWifi();

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  connectMQTT();

  Serial.println("\nConnected!");

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
    connectWifi();
    connectMQTT();
    client.loop();
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
                handleMovingState(stdDev, mean);
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
void handleMovingState(float stdDev, float currentMean) {
    Serial.printf("MOVING | Jitter: %.2f\n", stdDev);
    diodes(0xFF); 
    publishTelemetry("moving", -1, currentMean, stdDev);
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
    publishTelemetry("still", detectedFloor, meanPressure, 0.0);
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
/**
Connect wifi. 
*/
void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void connectMQTT() {
  if (client.connected()) return;

  Serial.println("Connecting to MQTT...");
  while (!client.connected()) {
    String client_id = "esp32-" + WiFi.macAddress();

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      client.subscribe(topic);
      client.publish(topic, "{\"type\":\"test\",\"status\":\"test\"}");
    } else {
      Serial.print("MQTT failed, state=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void publishTelemetry(const char* state, int floor, float mean, float stdev) {
    if (!client.connected()) return;
    float delta = mean - referencePressure;
    long uptime = millis() / 1000;
    int rssi = WiFi.RSSI();
    char jsonBuffer[512]; 
    snprintf(jsonBuffer, sizeof(jsonBuffer),
        "{"
          "\"device\": \"%s\","           // MAC address or ID
          "\"uptime_s\": %ld,"            // System uptime
          "\"wifi_rssi\": %d,"            // Signal strength
          "\"status\": \"%s\","           // "moving" or "still"
          "\"pressure\": {"
             "\"current\": %.2f,"         // Current window mean
             "\"ref\": %.2f,"             // Calibration baseline
             "\"delta\": %.2f,"           // Height relative to ground
             "\"jitter\": %.4f"           // Standard Deviation
          "},"
          "\"elevator\": {"
             "\"floor\": %d,"             // Detected floor (-1 if unknown)
             "\"mapped_offset\": %.2f"    // The learned offset for this floor
          "}"
        "}",
        WiFi.macAddress().c_str(),
        uptime,
        rssi,
        state,
        mean,
        referencePressure,
        delta,
        stdev,
        floor,
        (floor >= 0 && floor < NUM_FLOORS) ? FLOOR_OFFSETS[floor] : 0.0 
    );
    client.publish(topic, jsonBuffer);
}
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}



