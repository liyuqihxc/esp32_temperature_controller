cmake_minimum_required(VERSION 3.16)

set(SOUECE_FILES
  "src/func_trace.cc"
  "src/preferences.cc"
  "src/pid.cc"
  "src/m31865.cc"
)

idf_component_register(SRCS ${SOUECE_FILES}
  INCLUDE_DIRS "include"
  PRIV_REQUIRES app_trace spiffs nvs_flash driver)
