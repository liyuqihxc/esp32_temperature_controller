#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <esp_mac.h>
#include "app.h"
#include <lwip/ip4_addr.h>

CONST_STR(TAG_AP) = "WiFi-AP";
CONST_STR(TAG_STA) = "WiFi-Station";

esp_netif_t* s_esp_netif_ap = nullptr;
esp_event_handler_instance_t s_instance_ap_any_id = nullptr;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
const int max_wifi_connection_retry = 5;
esp_netif_t* s_esp_netif_station = nullptr;
esp_event_handler_instance_t s_instance_station_any_id = nullptr;
esp_event_handler_instance_t s_instance_station_got_ip = nullptr;
EventGroupHandle_t s_wifi_event_group = nullptr;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  FUNC_TRACE(wifi_event_handler);
  ESP_LOGI("wifi_event_handler", "event_base=%s, event_id=%ld", event_base, event_id);
  if (event_base == WIFI_EVENT)
  {
    if (s_esp_netif_ap != nullptr)
    {
      if (event_id == WIFI_EVENT_AP_STACONNECTED)
      {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG_AP, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
      }
      else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
      {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG_AP, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
      }
    }
    else if (s_esp_netif_station != nullptr)
    {
      int* retry_num = reinterpret_cast<int*>(arg);
      if (event_id == WIFI_EVENT_STA_START)
      {
        esp_wifi_connect();
      }
      else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
      {
        if (s_wifi_event_group != nullptr)
        {
          if (*retry_num > 0)
          {
            esp_wifi_connect();
            *retry_num -= 1;
            ESP_LOGI(TAG_STA, "retry to connect to the AP");
          }
          else
          {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
          }
          ESP_LOGI(TAG_STA, "connect to the AP fail");
        }
        else
        {
          esp_wifi_connect();
        }
      }
    }
  }
  else if (s_esp_netif_station != nullptr && event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG_STA, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    if (s_wifi_event_group != nullptr)
    {
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
  }
}

void wifi_init(void)
{
  FUNC_TRACE(wifi_init);
  ESP_ERROR_CHECK(esp_netif_init());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_event_loop_create_default();
}

void wifi_start_default_ap(void)
{
  FUNC_TRACE(wifi_start_default_ap);
  assert(s_esp_netif_ap == nullptr);
  ESP_LOGI(TAG_AP, "Starting default AP.");

  esp_netif_t* s_esp_netif_ap = esp_netif_create_default_wifi_ap();

  esp_netif_ip_info_t ipInfo;
  IP4_ADDR(&ipInfo.ip, 192,168,2,1);
  IP4_ADDR(&ipInfo.gw, 192,168,2,1);
  IP4_ADDR(&ipInfo.netmask, 255,255,255,0);
  esp_netif_dhcps_stop(s_esp_netif_ap);
  esp_netif_set_ip_info(s_esp_netif_ap, &ipInfo);
  esp_netif_dhcps_start(s_esp_netif_ap);

  ESP_LOGI(TAG_AP, "IP: " IPSTR, IP2STR(&ipInfo.ip));
  ESP_LOGI(TAG_AP, "Gateway: " IPSTR, IP2STR(&ipInfo.gw));
  ESP_LOGI(TAG_AP, "Netmask: " IPSTR, IP2STR(&ipInfo.netmask));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    ESP_EVENT_ANY_ID,
    &wifi_event_handler,
    NULL,
    &s_instance_ap_any_id));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  wifi_config_t wifi_config = {
    .ap = {
      .ssid = {},
      .password = {},
      .ssid_len = strlen(default_ap_ssid),
      .channel = 1,
      .authmode = WIFI_AUTH_WPA_WPA2_PSK,
      .max_connection = 4,
    },
  };
#pragma GCC diagnostic pop
  memcpy(wifi_config.ap.ssid, default_ap_ssid, strlen(default_ap_ssid) * sizeof(char));
  memcpy(wifi_config.ap.password, default_ap_password, strlen(default_ap_password) * sizeof(char));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG_AP, "start_default_ap finished. SSID:%s password:%s channel:%d", default_ap_ssid, default_ap_password, 1);
}

void wifi_stop_default_ap(void)
{
  FUNC_TRACE(wifi_stop_default_ap);
  ESP_ERROR_CHECK(esp_wifi_stop());
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  esp_netif_destroy(s_esp_netif_ap);
  s_esp_netif_ap = nullptr;
  esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, s_instance_ap_any_id);
  s_instance_ap_any_id = nullptr;
}

void wifi_connect_to_hotspot(const char* ssid, const char* password)
{
  FUNC_TRACE(wifi_connect_to_hotspot);
  assert(s_esp_netif_station == nullptr);
  ESP_LOGI(TAG_STA, "Attempting to connect to hotspot %s.", ssid);

  s_wifi_event_group = xEventGroupCreate();
  s_esp_netif_station = esp_netif_create_default_wifi_sta();
  int num_retry = max_wifi_connection_retry;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    ESP_EVENT_ANY_ID,
    &wifi_event_handler,
    &num_retry,
    &s_instance_station_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    IP_EVENT_STA_GOT_IP,
    &wifi_event_handler,
    NULL,
    &s_instance_station_got_ip));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  wifi_config_t wifi_config = {
    .sta = {
      .ssid = {},
      .password = {},
      .threshold = {
        .authmode = WIFI_AUTH_WPA2_PSK,
      },
      .pmf_cfg = {
        .capable = true,
        .required = false
      },
    },
  };
#pragma GCC diagnostic pop
  strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid);
  strcpy(reinterpret_cast<char*>(wifi_config.sta.password), password);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
  if (bits & WIFI_CONNECTED_BIT)
  {
    ESP_LOGI(TAG_STA, "connected to ap SSID:%s", ssid);
  }
  else if (bits & WIFI_FAIL_BIT)
  {
    ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s", ssid);
  }
  else
  {
    ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
  }
  vEventGroupDelete(s_wifi_event_group);
  s_wifi_event_group = nullptr;
}

void wifi_disconnect(void)
{
  FUNC_TRACE(wifi_disconnect);
  ESP_ERROR_CHECK(esp_wifi_disconnect());
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  esp_netif_destroy(s_esp_netif_station);
  s_esp_netif_station = nullptr;
  esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, s_instance_station_any_id);
  s_instance_station_any_id = nullptr;
  esp_event_handler_instance_unregister(WIFI_EVENT, IP_EVENT_STA_GOT_IP, s_instance_station_got_ip);
  s_instance_station_got_ip = nullptr;
}
