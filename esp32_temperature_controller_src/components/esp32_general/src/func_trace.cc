#include "esp_log.h"
#include "esp32_general.h"

func_tracer::func_tracer(const char* func) : m_func(func)
{
  ESP_LOGD("Func Trace", "Entered %s()", func);
}

func_tracer::~func_tracer()
{
  ESP_LOGD("Func Trace", "Leaving %s()", m_func);
}
