#ifndef _ESP32_GENERAL_PREFERENCE_H_
#define _ESP32_GENERAL_PREFERENCE_H_

#ifndef _ESP32_GENERAL_H_
#include "esp_err.h"
#endif

class Preferences
{
protected:
  const char *const TAG = "Preferences";
  uint32_t _handle;
  bool _started;
  bool _readOnly;
public:
  Preferences();
  ~Preferences();
  Preferences(Preferences&) = delete;
  Preferences& operator=(const Preferences&) = delete;
public:
  esp_err_t open(const char* name, bool readOnly=false, const char* partition_label=nullptr);
  void close(void);
  bool clear(void);
  bool remove(const char* key);
public:
  template<typename T>
  size_t put_number(const char* key, const T data);
  size_t put_str(const char* key, const char* data);
  template<typename T>
  T get_number(const char* key, const T defaultValue);
  bool get_str(const char* key, char* value, size_t* value_len, const char* defaultValue);
  //size_t put_blob();
};

#endif
