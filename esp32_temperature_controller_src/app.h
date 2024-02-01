#ifndef _APP_H_
#define _APP_H_

#include <Preferences.h>

#define _DEBUG

#ifdef _DEBUG
#define SERIAL_WRITE(s) Serial.print(s)
#define SERIAL_WRITE_LINE(s) Serial.println(s)
#else
#define SERIAL_WRITE(s)
#define SERIAL_WRITE_LINE(s)
#endif

#define PIN_DHT20_SCL         0
#define PIN_DHT20_SDA         2

#define PIN_TEMP1_CS          3
#define PIN_TEMP2_CS          5
#define PIN_TEMP_DI           1
#define PIN_TEMP_DO           19
#define PIN_TEMP_CLK          21
#define PIN_TEMP1_RDY         22
#define PIN_TEMP2_RDY         16

#define PIN_ZERO_CROSS        10
#define PIN_HEATER_CTRL       11
#define PIN_LED_HEATING_STATE 23

const char *const PREF_APP_NAME = "name";
const char *const PREF_HEATER_MODE = "mode";
const char *const PREF_HEATER_VALUE = "value";
const char *const PREF_WIFI_SSID = "ssid";
const char *const PREF_WIFI_PASSWORD = "password";
const char *const PREF_ENABLE_MQTT_CLIENT = "mqtt_on";
const char *const PREF_MQTT_BROKER = "broker";
const char *const PREF_MQTT_TOPIC = "topic";

// Default AP credentials
const char *const default_ap_ssid = "ESP32_TEMP_CTRL";
const char *const default_ap_password = "12345678";

extern Preferences preferences;

#endif
