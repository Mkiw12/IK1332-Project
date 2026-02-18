#include <Wire.h>
#include <SparkFun_BMP581_Arduino_Library.h>
#include "dsplp_io.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ============================
// -------- SETTINGS ----------
// ============================

// WiFi
const char *ssid = "Jacob";
const char *password = "jacob12345";

// MQTT
const char *mqtt_broker = "jf9611d6.ala.eu-central-1.emqxsl.com";
const char *mqtt_username = "test";
const char *mqtt_password = "test";
const int mqtt_port = 8883;

const char* TOPIC_DATA = "ik1332/proj/sensors/hiss1/data";
const char* TOPIC_MAP  = "ik1332/proj/sensors/hiss1/map";

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

// ============================
// -------- SENSOR ------------
// ============================

BMP581 pressureSensor;
uint8_t i2cAddress = BMP581_I2C_ADDRESS_DEFAULT;

// Moving window
const int WINDOW_SIZE = 20;
float historyBuffer[WINDOW_SIZE];
int bufferIndex = 0;
bool bufferFull = false;

float referencePressure = 0;
bool mapLoaded = false;

// Floor map
float FLOOR_OFFSETS[] = {0, -39.6733, -75.8909, -113.5566, -151.4735, -185.2196};
const int NUM_FLOORS = sizeof(FLOOR_OFFSETS) / sizeof(FLOOR_OFFSETS[0]);
const float FLOOR_THRESHOLD = 10.0;
const float JITTER_THRESHOLD = 1.5;

// Telemetry state
unsigned long lastMsgTime = 0;
const long interval = 1000;

const char* currentTrajectory = "unknown";
int currentFloor = -1;
float currentMean = 0;
float currentStdev = 0;

// Networking
WiFiClientSecure espClient;
PubSubClient client(espClient);

// ============================
// -------- PROTOTYPES --------
// ============================

void connectWifi();
void connectMQTT();
void publishTelemetry();
void publishMap();
void callback(char *topic, byte *payload, unsigned int length);
void handleMovingState();
void handleStillState();
void diodes(uint8_t leds);

// ============================
// -------- SETUP -------------
// ============================

void setup() {
  Serial.begin(115200);
  Wire.begin(2, 1);

  espClient.setCACert(ca_cert);

  connectWifi();
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  connectMQTT();

  pinMode(LED_SDA_IO, OUTPUT);
  pinMode(LED_SHCP_IO, OUTPUT);
  pinMode(LED_STCP_IO, OUTPUT);

  if (pressureSensor.beginI2C(i2cAddress) != BMP5_OK) {
    Serial.println("Sensor error.");
    while (1);
  }

  // Wait for retained map
  unsigned long startWait = millis();
  while (!mapLoaded && millis() - startWait < 3000) {
    client.loop();
  }

  // If no retained map, calibrate
  if (!mapLoaded) {
    Serial.println("No retained map found. Calibrating...");

    bmp5_sensor_data data;
    float sum = 0;

    for (int i = 0; i < 100; i++) {
      pressureSensor.getSensorData(&data);
      sum += data.pressure;
      delay(10);
    }

    referencePressure = sum / 100.0;
    publishMap();
  }
  else {
    Serial.println("Using retained map.");
  }
}

// ============================
// -------- LOOP --------------
// ============================

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
      for (int i = 0; i < WINDOW_SIZE; i++)
        sum += historyBuffer[i];

      currentMean = sum / WINDOW_SIZE;

      float var = 0;
      for (int i = 0; i < WINDOW_SIZE; i++)
        var += pow(historyBuffer[i] - currentMean, 2);

      currentStdev = sqrt(var / WINDOW_SIZE);

      if (currentStdev > JITTER_THRESHOLD)
        handleMovingState();
      else
        handleStillState();
    }
  }

  if (millis() - lastMsgTime > interval) {
    lastMsgTime = millis();
    publishTelemetry();
  }
}

// ============================
// -------- STATES ------------
// ============================

void handleMovingState() {
  currentTrajectory = "moving";
  diodes(0xFF);
}

void handleStillState() {

  currentTrajectory = "still";
  float currentDelta = currentMean - referencePressure;
  int detectedFloor = -1;
  float learningRate = 0.02;

  for (int i = 0; i < NUM_FLOORS; i++) {
    if (abs(currentDelta - FLOOR_OFFSETS[i]) < FLOOR_THRESHOLD) {
      detectedFloor = i;
      FLOOR_OFFSETS[i] =
        FLOOR_OFFSETS[i] * (1 - learningRate) +
        currentDelta * learningRate;
      break;
    }
  }

  currentFloor = detectedFloor;

  if (detectedFloor != -1)
    diodes(~(1 << detectedFloor));
  else
    diodes(0x00);
}

// ============================
// -------- MQTT --------------
// ============================

void publishTelemetry() {

  if (!client.connected()) return;

  StaticJsonDocument<256> doc;

  doc["traj"] = currentTrajectory;
  doc["height"] = currentMean - referencePressure;
  doc["floor"] = currentFloor;
  doc["rssi"] = WiFi.RSSI();

  char buffer[256];
  serializeJson(doc, buffer);

  client.publish(TOPIC_DATA, buffer, false);
}

void publishMap() {

  StaticJsonDocument<512> doc;

  doc["ref"] = referencePressure;

  JsonArray arr = doc.createNestedArray("offsets");
  for (int i = 0; i < NUM_FLOORS; i++)
    arr.add(FLOOR_OFFSETS[i]);

  char buffer[512];
  serializeJson(doc, buffer);

  client.publish(TOPIC_MAP, buffer, true); // retained
}

void callback(char *topic, byte *payload, unsigned int length) {

  if (strcmp(topic, TOPIC_MAP) != 0) return;

  StaticJsonDocument<512> doc;

  DeserializationError err =
    deserializeJson(doc, payload, length);

  if (err) {
    Serial.println("JSON parse failed.");
    return;
  }
  referencePressure = doc["ref"];
  JsonArray arr = doc["offsets"];
  for (int i = 0; i < NUM_FLOORS && i < arr.size(); i++)
    FLOOR_OFFSETS[i] = arr[i];

  mapLoaded = true;
  Serial.println("Retained map restored.");
}

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
}

void connectMQTT() {
  if (client.connected()) return;

  while (!client.connected()) {

    String client_id = "esp32-" + WiFi.macAddress();

    if (client.connect(client_id.c_str(),
                       mqtt_username,
                       mqtt_password)) {
      client.subscribe(TOPIC_MAP);
    }
    else {
      delay(2000);
    }
  }
}

// ============================
// -------- LED DRIVER --------
// ============================

void diodes(uint8_t leds) {

  for (int led = 0; led < 8; led++) {
    digitalWrite(LED_SDA_IO, (leds & (1 << led)) ? HIGH : LOW);
    digitalWrite(LED_SHCP_IO, HIGH);
    delayMicroseconds(1);
    digitalWrite(LED_SHCP_IO, LOW);
  }

  digitalWrite(LED_STCP_IO, HIGH);
  delayMicroseconds(1);
  digitalWrite(LED_STCP_IO, LOW);
}
