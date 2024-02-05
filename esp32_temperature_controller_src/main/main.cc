#include <nvs_flash.h>
#include <esp_spiffs.h>
#include <esp_event.h>
#include "app.h"
#include <driver/uart.h>

Preferences s_preferences;

extern "C" void app_main(void)
{
  FUNC_TRACE(app_main);
  ESP_ERROR_CHECK(nvs_flash_init());
  s_preferences.open("app");
  // s_preferences.clear();

  esp_vfs_spiffs_conf_t conf =
  {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
  };
  ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

  wifi_init();

  {
    size_t length = 0;
    s_preferences.get_str(PREF_WIFI_SSID, nullptr, &length, "");
    if (length == 0)
    {
      wifi_start_default_ap();
      web_setup_config_server();
    }
    else
    {
      char ssid[length] = {};
      s_preferences.get_str(PREF_WIFI_SSID, ssid, &length, "");
      length = 0;
      s_preferences.get_str(PREF_WIFI_PASSWORD, nullptr, &length, "");
      char password[length] = {};
      s_preferences.get_str(PREF_WIFI_PASSWORD, password, &length, "");
      ESP_LOGI("app", "ssid %s, password %s", ssid, password);
      wifi_connect_to_hotspot(ssid, password);

      web_setup_server();
      temp_sensor_init();
    }
  }
}
