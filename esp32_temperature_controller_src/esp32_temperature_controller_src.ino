extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncMqttClient.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <PID_v1.h>
#include "app.h"
#include "PID_AutoTune_v0.h"
#include "DHT20.h"
#include "M31865.h"

// https://github.com/br3ttb/Arduino-PID-AutoTune-Library
// https://gist.github.com/weshouman/11e291dd4fe52422c89522ce485e29f3#setting-up-an-spiffs

Preferences preferences;
DHT20 dht20;
AsyncWebServer server(80);
extern void RegisterConfigWebRouter(AsyncWebServer& server);
extern void RegisterWebRouter(AsyncWebServer& server);

void setup()
{
  Serial.begin(115200);
  preferences.begin("app");
  if (preferences.getString(PREF_WIFI_SSID, "") == "")
  {
    WiFi.softAP(default_ap_ssid, default_ap_password);
    RegisterConfigWebRouter(server);
    server.begin();
    SERIAL_WRITE_LINE("Web server started.");
    return;
  }
  //attachInterrupt(digitalPinToInterrupt(PIN_ZERO_CROSS), function, LOW);// detect zero cross for ac phase
  Wire.begin(PIN_DHT20_SDA, PIN_DHT20_SCL);
  SPI.begin(PIN_TEMP_CLK, PIN_TEMP_DO, PIN_TEMP_DI, PIN_TEMP1_CS);
  //digitalWrite(PIN_TEMP2_CS, HIGH);
  //attachInterrupt(digitalPinToInterrupt(PIN_TEMP1_RDY), function, LOW);
  //attachInterrupt(digitalPinToInterrupt(PIN_TEMP2_RDY), function, LOW);

  // Connect to Wi-Fi
  const String ssid = preferences.getString(PREF_WIFI_SSID, "");
  const String password = preferences.getString(PREF_WIFI_PASSWORD, "");
  SERIAL_WRITE("Connecting to hotspot");
  SERIAL_WRITE_LINE(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    SERIAL_WRITE_LINE("Retrying to connect to hotspot");
  }

  SERIAL_WRITE_LINE(WiFi.localIP());

  // Start server
  RegisterWebRouter(server);
  server.begin();
  SERIAL_WRITE_LINE("Web server started.");
}

void loop()
{

}
