#include <esp_http_server.h>
#include <esp_spiffs.h>
#include <esp_log.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "app.h"

CONST_STR(HTTP_CONTENT_HTML) = "text/html";
CONST_STR(HTTP_HEADER_LOCATION) = "Location";
CONST_STR(HTTP_STATUS_203_NO_CONTENT) = "203";
CONST_STR(HTTP_STATUS_307_TEMPORARY_REDIRECT) = "307";
CONST_STR(HTTP_STATUS_400_BAD_REQUEST) = "400";

CONST_STR(INDEX_HTML_PATH) = "/spiffs/index.html";
CONST_STR(TAG) = "Web";
httpd_handle_t web_server_handle = nullptr;

void url_decode(const char* raw, char* str);

esp_err_t get_config_index_page_handler(httpd_req_t*);
esp_err_t get_config_config_page_handler(httpd_req_t*);
esp_err_t post_api_config_handler(httpd_req_t*);
esp_err_t delete_api_preferences_handler(httpd_req_t*);
esp_err_t get_index_page_handler(httpd_req_t*);

httpd_uri_t config_index_page = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = get_config_index_page_handler,
  .user_ctx = NULL
};

httpd_uri_t config_config_page = {
  .uri = "/config",
  .method = HTTP_GET,
  .handler = get_config_config_page_handler,
  .user_ctx = NULL
};

httpd_uri_t api_config_save = {
  .uri = "/api/config",
  .method = HTTP_POST,
  .handler = post_api_config_handler,
  .user_ctx = NULL
};

httpd_uri_t api_preferences_clear = {
  .uri = "/api/preferences",
  .method = HTTP_DELETE,
  .handler = delete_api_preferences_handler,
  .user_ctx = NULL
};

httpd_uri_t index_page = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = get_index_page_handler,
  .user_ctx = NULL
};

FILE* open_web_page(const char* path, struct stat* st)
{
  FUNC_TRACE(open_web_page);

  if (stat(path, st))
  {
    ESP_LOGE(TAG, "index.html not found");
    return nullptr;
  }

  return fopen(path, "r");
}

void web_setup_config_server(void)
{
  FUNC_TRACE(web_setup_config_server);
  assert(web_server_handle == nullptr);
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  ESP_LOGI(TAG, "Starting web server for setup.");
  esp_err_t err = httpd_start(&web_server_handle, &config);
  if (err == ESP_OK)
  {
    httpd_register_uri_handler(web_server_handle, &config_index_page);
    httpd_register_uri_handler(web_server_handle, &config_config_page);
    httpd_register_uri_handler(web_server_handle, &api_config_save);
    httpd_register_uri_handler(web_server_handle, &api_preferences_clear);
  }
  else
  {
    ESP_LOGE(TAG, "httpd_start failed %d", err);
  }
}

void web_setup_server(void)
{
  FUNC_TRACE(web_setup_server);
  ESP_LOGI(TAG, "Starting web server.");
  assert(web_server_handle == nullptr);
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  esp_err_t err = httpd_start(&web_server_handle, &config);
  if (err == ESP_OK)
  {
    httpd_register_uri_handler(web_server_handle, &index_page);
    httpd_register_uri_handler(web_server_handle, &api_preferences_clear);
  }
  else
  {
    ESP_LOGE(TAG, "httpd_start failed %d", err);
  }
}

esp_err_t get_config_index_page_handler(httpd_req_t* req)
{
  FUNC_TRACE(get_config_index_page_handler);
  httpd_resp_set_status(req, HTTP_STATUS_307_TEMPORARY_REDIRECT);
  httpd_resp_set_hdr(req, HTTP_HEADER_LOCATION, "/config");
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

esp_err_t get_config_config_page_handler(httpd_req_t* req)
{
  FUNC_TRACE(get_config_config_page_handler);
  return get_index_page_handler(req);
}

esp_err_t post_api_config_handler(httpd_req_t* req)
{
  FUNC_TRACE(post_api_config_handler);
  int raw_query_len = httpd_req_get_url_query_len(req) + 1;
  if (raw_query_len <= 21 + 1)
  {
    httpd_resp_set_status(req, HTTP_STATUS_400_BAD_REQUEST);
    ESP_LOGE(TAG, "Invalid request for /api/config.");
    httpd_resp_send(req, nullptr, 0);
    return ESP_ERR_HTTPD_INVALID_REQ;
  }
  char query[raw_query_len];
  {
    char buffer[raw_query_len];
    if (httpd_req_get_url_query_str(req, buffer, raw_query_len) != ESP_OK)
    {
      httpd_resp_send_500(req);
      ESP_LOGE(TAG, "Failed to get query string.");
      return ESP_ERR_HTTPD_INVALID_REQ;
    }
    ESP_LOGI(TAG, "HTTP GET /api/config %s", buffer);
    memset(query, 0, raw_query_len);
    url_decode(buffer, query);
    ESP_LOGD(TAG, "decoded HTTP GET /api/config %s", query);
  }

  char buffer[31+33+64];
  char* name = buffer, *ssid = &buffer[31], *password = &buffer[31+33];
  memset(buffer, 0, 128);
  for (char* i = query, *str = query; i <= query + raw_query_len; i++)
  {
    if (*i == '&' || i == &query[raw_query_len])
    {
      *i = '\0';
      if (strncmp(str, PREF_APP_NAME, 4) == 0)
      {
        str += 5;
        if (strlen(str) > 30)
        {
          httpd_resp_set_status(req, HTTP_STATUS_400_BAD_REQUEST);
          ESP_LOGE(TAG, "Invalid request for /api/config, name can only be 30 characters.");
          httpd_resp_send(req, "name can only be 30 characters.", HTTPD_RESP_USE_STRLEN);
          return ESP_ERR_HTTPD_INVALID_REQ;
        }
        strcpy(name, str);
      }
      else if (strncmp(str, PREF_WIFI_SSID, 4) == 0)
      {
        str += 5;
        int val_len = strlen(str);
        if (val_len < 2 || val_len > 32)
        {
          httpd_resp_set_status(req, HTTP_STATUS_400_BAD_REQUEST);
          if (val_len == 0)
          {
            ESP_LOGE(TAG, "Invalid request for /api/config, ssid is required parameter.");
            httpd_resp_send(req, "ssid is required parameter.", HTTPD_RESP_USE_STRLEN);
          }
          else
          {
            ESP_LOGE(TAG, "Invalid request for /api/config, ssid can only be 2 to 32 characters.");
            httpd_resp_send(req, "ssid can only be 2 to 32 characters.", HTTPD_RESP_USE_STRLEN);
          }
          return ESP_ERR_HTTPD_INVALID_REQ;
        }
        strcpy(ssid, str);
      }
      else if (strncmp(str, PREF_WIFI_PASSWORD, 8) == 0)
      {
        str += 9;
        int val_len = strlen(str);
        if (val_len < 8 || val_len > 63)
        {
          httpd_resp_set_status(req, HTTP_STATUS_400_BAD_REQUEST);
          if (val_len == 0)
          {
            ESP_LOGE(TAG, "Invalid request for /api/config, password is required parameter.");
            httpd_resp_send(req, "password is required parameter.", HTTPD_RESP_USE_STRLEN);
          }
          else
          {
            ESP_LOGE(TAG, "Invalid request for /api/config, password can only be 8 to 63 characters.");
            httpd_resp_send(req, "password can only be 8 to 63.", HTTPD_RESP_USE_STRLEN);
          }
          return ESP_ERR_HTTPD_INVALID_REQ;
        }
        strcpy(password, str);
      }

      str = i + 1;
    }
    else
    {
      continue;
    }
  }

  if (*name != '\0')
    s_preferences.put_str(PREF_APP_NAME, name);
  s_preferences.put_str(PREF_WIFI_SSID, ssid);
  s_preferences.put_str(PREF_WIFI_PASSWORD, password);
  memset(buffer, 0, 128);
  size_t len = 11;
  s_preferences.get_str(PREF_WIFI_PASSWORD, password, &len, "");
  ESP_LOGI(TAG, "name=%s, ssid=%s, password=%s.", name, ssid, password);

  wifi_stop_default_ap();
  esp_restart();

  return ESP_OK;
}

esp_err_t delete_api_preferences_handler(httpd_req_t* req)
{
  FUNC_TRACE(delete_api_preferences_handler);
  s_preferences.clear();
  httpd_resp_set_status(req, HTTP_STATUS_203_NO_CONTENT);
  ESP_LOGE(TAG, "Preferences cleared.");
  httpd_resp_send(req, nullptr, 0);
  return ESP_OK;
}

esp_err_t get_index_page_handler(httpd_req_t* req)
{
  FUNC_TRACE(get_index_page_handler);
  httpd_resp_set_type(req, HTTP_CONTENT_HTML);
  struct stat st;
  FILE* file = open_web_page(INDEX_HTML_PATH, &st);
  if (file == nullptr)
  {
    ESP_LOGE(TAG, "Failed to open file %s", INDEX_HTML_PATH);
    httpd_resp_send_404(req);
    return ESP_ERR_NOT_FOUND;
  }
  char buff[1024];
  ESP_LOGD(TAG, "Start transfering file %s", INDEX_HTML_PATH);
  while(true)
  {
    size_t size = fread(buff, 1, 1024, file);
    if (size == 0)
    {
      ESP_LOGD(TAG, "EOF file %s", INDEX_HTML_PATH);
      break;
    }
    esp_err_t ret = httpd_resp_send_chunk(req, buff, size);
    if (ret != ESP_OK)
    {
      ESP_LOGE(TAG, "httpd_resp_send_chunk() failed %d", ret);
      break;
    }
  }
  return httpd_resp_send_chunk(req, NULL, 0);
}

void url_decode(const char* raw, char* str)
{
  FUNC_TRACE(url_decode);
  char temp[] = "0x00";
  int raw_len = strlen(raw);
  for (int i = 0, target_index = 0; i < raw_len; target_index++)
  {
    char decodedChar;
    char encodedChar = raw[i++];
    if ((encodedChar == '%') && (i + 1 < raw_len)){
      temp[2] = raw[i++];
      temp[3] = raw[i++];
      decodedChar = strtol(temp, NULL, 16);
    } else if (encodedChar == '+') {
      decodedChar = ' ';
    } else {
      decodedChar = encodedChar;  // normal ascii char
    }
    str[target_index] = decodedChar;
  }
}
