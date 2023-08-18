#include "wifiota.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ElegantOTA.h>

const char* ssid = "IOT3";
const char* password = "deadbeef12345";
WebServer server(80);

void setupWifi(){
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.hostname("buttonbox.local");
  WiFi.begin(ssid, password);
  xTaskCreatePinnedToCore ( otaTask,"ota",2000,NULL,0,NULL,0 );
}

void otaTask (void* pvParameters) {
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(250);
  }
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println(WiFi.getHostname());

  while(1) {
server.handleClient();  }
}
