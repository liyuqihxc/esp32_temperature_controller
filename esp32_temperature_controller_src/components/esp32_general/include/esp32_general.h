#ifndef _ESP32_GENERAL_H_
#define _ESP32_GENERAL_H_

#include "esp_err.h"
#include "preferences.h"

class func_tracer
{
private:
  const char *const m_func;
public:
  func_tracer(const char* func);
  ~func_tracer();
  func_tracer(func_tracer&) = delete;
  func_tracer& operator=(const func_tracer&) = delete;
};

#define FUNC_TRACE(fn) \
  func_tracer __func_tracer__(#fn)



#endif
