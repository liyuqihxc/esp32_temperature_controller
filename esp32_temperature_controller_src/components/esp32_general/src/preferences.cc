#include <type_traits>
#include <string.h>
#include <esp_err.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp32_general.h"

Preferences::Preferences()
    :_handle(0)
    ,_started(false)
    ,_readOnly(false)
{FUNC_TRACE(Preferences::Preferences);}

Preferences::~Preferences()
{
  FUNC_TRACE(Preferences::~Preferences);
  close();
}

esp_err_t Preferences::open(const char* name, bool readOnly, const char* partition_label)
{
  FUNC_TRACE(Preferences::open);
  if(_started)
  {
    return false;
  }
  _readOnly = readOnly;
  esp_err_t err = ESP_OK;
  if (partition_label != nullptr)
  {
    err = nvs_flash_init_partition(partition_label);
    if (err)
    {
      ESP_LOGE(TAG, "nvs_flash_init_partition failed: %d", err);
      return err;
    }
    err = nvs_open_from_partition(partition_label, name, readOnly ? NVS_READONLY : NVS_READWRITE, &_handle);
  }
  else
  {
    err = nvs_open(name, readOnly ? NVS_READONLY : NVS_READWRITE, &_handle);
  }
  if(err)
  {
    ESP_LOGE(TAG, "nvs_open failed: %d", err);
    return err;
  }
  _started = true;
  return err;
}

void Preferences::close(void)
{
  FUNC_TRACE(Preferences::close);
}

bool Preferences::clear(void)
{
  FUNC_TRACE(Preferences::clear);
  if(!_started || _readOnly)
  {
    return false;
  }
  esp_err_t err = nvs_erase_all(_handle);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_erase_all fail: %d", err);
    return false;
  }
  err = nvs_commit(_handle);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_commit fail: %d", err);
    return false;
  }
  return true;
}

bool Preferences::remove(const char* key)
{
  FUNC_TRACE(Preferences::remove);
  if(!_started || !key || _readOnly)
  {
    return false;
  }
  esp_err_t err = nvs_erase_key(_handle, key);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_erase_key fail: %s %d", key, err);
    return false;
  }
  err = nvs_commit(_handle);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
    return false;
  }
  return true;
}

template<typename T>
size_t Preferences::put_number(const char* key, const T data)
{
  static_assert(std::is_arithmetic_v<T>, "");
  if(!_started || !key || _readOnly)
  {
    return 0;
  }
  esp_err_t err = ESP_OK;
  if constexpr (std::is_same_v<T, uint8_t>)
    err = nvs_set_u8(_handle, key, &data);
  else if (std::is_same_v<T, int8_t>)
    err = nvs_set_i8(_handle, key, &data);
  else if (std::is_same_v<T, uint16_t>)
    err = nvs_set_u16(_handle, key, &data);
  else if (std::is_same_v<T, int16_t>)
    err = nvs_set_i16(_handle, key, &data);
  else if (std::is_same_v<T, uint32_t>)
    err = nvs_set_u32(_handle, key, &data);
  else if (std::is_same_v<T, int32_t>)
    err = nvs_set_i32(_handle, key, &data);
  else if (std::is_same_v<T, uint64_t>)
    err = nvs_set_u64(_handle, key, &data);
  else if (std::is_same_v<T, int64_t>)
    err = nvs_set_i64(_handle, key, &data);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_set_* fail: %s %d", key, err);
    return 0;
  }
  err = nvs_commit(_handle);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
    return 0;
  }
  return sizeof(data);
}

size_t Preferences::put_str(const char* key, const char* data)
{
  if(!_started || !key || _readOnly) {
    return 0;
  }
  esp_err_t err = nvs_set_str(_handle, key, data);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_set_str fail: %s %d", key, err);
    return 0;
  }
  err = nvs_commit(_handle);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_commit fail: %s %d", key, err);
    return 0;
  }
  return strlen(data);
}

template<typename T>
T Preferences::get_number(const char* key, const T defaultValue)
{
  static_assert(std::is_arithmetic_v<T>, "");
  T value = defaultValue;
  if(!_started || !key)
  {
    return 0;
  }

  esp_err_t err = ESP_OK;
  if constexpr (std::is_same_v<T, uint8_t>)
    err = nvs_get_u8(_handle, key, &value);
  else if (std::is_same_v<T, int8_t>)
    err = nvs_get_i8(_handle, key, &value);
  else if (std::is_same_v<T, uint16_t>)
    err = nvs_get_u16(_handle, key, &value);
  else if (std::is_same_v<T, int16_t>)
    err = nvs_get_i16(_handle, key, &value);
  else if (std::is_same_v<T, uint32_t>)
    err = nvs_get_u32(_handle, key, &value);
  else if (std::is_same_v<T, int32_t>)
    err = nvs_get_i32(_handle, key, &value);
  else if (std::is_same_v<T, uint64_t>)
    err = nvs_get_u64(_handle, key, &value);
  else if (std::is_same_v<T, int64_t>)
    err = nvs_get_i64(_handle, key, &value);

  if(err)
  {
    ESP_LOGE(TAG, "nvs_get_* fail: %s %d", key, err);
    return 0;
  }
  return sizeof(T);
}

bool Preferences::get_str(const char* key, char* value, size_t* value_len, const char* defaultValue)
{
  FUNC_TRACE(Preferences::get_str);
  if(!_started || !key)
  {
    return false;
  }

  size_t str_len = 0;
  esp_err_t err = nvs_get_str(_handle, key, nullptr, &str_len);
  if(err)
  {
    ESP_LOGE(TAG, "nvs_get_str fail: %s %d", key, err);
  }
  else if (value == nullptr)
  {
    *value_len = str_len;
    return true;
  }
  else if (value != nullptr && *value_len >= str_len)
  {
    nvs_get_str(_handle, key, value, value_len);
    return true;
  }

  if (value != nullptr && *value_len >= strlen(defaultValue))
    strcpy(value, defaultValue);
  return false;
}
