#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "STARLINK";
const char* password = ""; // Sua senha

void setup() {
  Serial.begin(115200);
  
  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected! IP: ");
  Serial.println(WiFi.localIP());
  
  // OTA
  ArduinoOTA.setHostname("esp32-test-ota");
  ArduinoOTA.onStart([]() {
    Serial.println("OTA: Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA: End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void loop() {
  ArduinoOTA.handle();
  
  // Debug: mostra que está funcionando
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) {
    Serial.println("Loop executando...");
    lastDebug = millis();
  }
}