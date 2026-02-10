#include <WiFi.h>

void setup() {
  Serial.begin(115200); 
  
  Serial.print("Connecting to WiFi");
  WiFi.begin("Tele2_A74B76", "2egdhpw3");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("My IP Address: ");
  Serial.println(WiFi.localIP()); 
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();

    if (c == 'A') {
      for (int k = 0; k < 5; k++) {
        Serial.print("PRINTERING local IP: ");
        Serial.println(WiFi.localIP());
      }
      Serial.println("SS"); 
  }
}