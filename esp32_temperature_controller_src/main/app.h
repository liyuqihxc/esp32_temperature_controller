#ifndef _APP_H_
#define _APP_H_

#include "esp32_general.h"
#include <esp_err.h>
#include <esp_log.h>

#define PIN_DHT20_SCL         0
#define PIN_DHT20_SDA         2

// #define PIN_TEMP1_CS          3
#define PIN_TEMP1_CS          17
#define PIN_TEMP2_CS          5
// #define PIN_TEMP_MOSI         1
#define PIN_TEMP_MOSI         18
#define PIN_TEMP_MISO         19
#define PIN_TEMP_CLK          21
#define PIN_TEMP1_RDY         22
#define PIN_TEMP2_RDY         16

#define PIN_ZERO_CROSS        10
#define PIN_HEATER_CTRL       11
#define PIN_LED_HEATING_STATE 23

#define CONST_STR(v) \
  constexpr const char* v

constexpr u_int16_t M31865_Rref = 430;
CONST_STR(TEMP_SENSOR_EVENT) = "TEMP_SENSOR_EVENT";
typedef enum
{
  RTD_READOUT,
  RTD_FAULT
}temp_sensor_event;

CONST_STR(PREF_APP_NAME) = "name";
CONST_STR(PREF_HEATER_MODE) = "mode";
CONST_STR(PREF_HEATER_VALUE) = "value";
CONST_STR(PREF_WIFI_SSID) = "ssid";
CONST_STR(PREF_WIFI_PASSWORD) = "password";
CONST_STR(PREF_ENABLE_MQTT_CLIENT) = "mqtt_on";
CONST_STR(PREF_MQTT_BROKER) = "broker";
CONST_STR(PREF_MQTT_TOPIC) = "topic";

// Default AP credentials
CONST_STR(default_ap_ssid) = "ESP32_TEMP_CTRL";
CONST_STR(default_ap_password) = "12345678";

void wifi_init(void);
void wifi_start_default_ap(void);
void wifi_stop_default_ap(void);
void wifi_connect_to_hotspot(const char* ssid, const char* password);
void wifi_disconnect(void);

void web_setup_config_server(void);
void web_setup_server(void);

void temp_sensor_init(void);

// esp_err_t bluetooth_spp_start(void);
// void bluetooth_spp_redirect_log(void);

extern Preferences s_preferences;

#endif
