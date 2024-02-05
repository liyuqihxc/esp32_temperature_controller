#include <hal/spi_types.h>
#include <driver/spi_master.h>
#include <freertos/task.h>
#include <m31865.h>
#include "app.h"

CONST_STR(TAG) = "TEMP_SENSOR";
spi_device_handle_t s_m31865_1 = nullptr;
spi_device_handle_t s_m31865_2 = nullptr;
TaskHandle_t s_task_handle;

void temp_sensor_proc(void*)
{
  while (true)
  {
    spi_device_acquire_bus(s_m31865_1, portMAX_DELAY);
    uint16_t rtd = m31865_read_rtd(s_m31865_1);
    float ratio = rtd;
    ratio /= 32768;
    ESP_LOGI(TAG, "Ratio=%.8f, Resistance=%.8f, Temperature=%.2f", ratio, M31865_Rref * ratio, calculate_temperature(rtd, 100, 430));
    uint8_t fault = m31865_read_fault(s_m31865_1);
    if (fault)
    {
      ESP_LOGW(TAG, "Fault %x", fault);
      if (fault & MAX31865_FAULT_HIGHTHRESH) {
        ESP_LOGW(TAG, "RTD High Threshold");
      }
      if (fault & MAX31865_FAULT_LOWTHRESH) {
        ESP_LOGW(TAG, "RTD Low Threshold");
      }
      if (fault & MAX31865_FAULT_REFINLOW) {
        ESP_LOGW(TAG, "REFIN- > 0.85 x Bias");
      }
      if (fault & MAX31865_FAULT_REFINHIGH) {
        ESP_LOGW(TAG, "REFIN- < 0.85 x Bias - FORCE- open");
      }
      if (fault & MAX31865_FAULT_RTDINLOW) {
        ESP_LOGW(TAG, "RTDIN- < 0.85 x Bias - FORCE- open");
      }
      if (fault & MAX31865_FAULT_OVUV) {
        ESP_LOGW(TAG, "Under/Over voltage");
      }
      m31865_clear_fault(s_m31865_1);
    }
    spi_device_release_bus(s_m31865_1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(nullptr);
}

void temp_sensor_init(void)
{
  spi_bus_config_t buscfg =
  {
    .mosi_io_num = PIN_TEMP_MOSI,
    .miso_io_num = PIN_TEMP_MISO,
    .sclk_io_num = PIN_TEMP_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 0
  };
  esp_err_t ret=spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to initialize spi2 interface, spi_bus_initialize %d", ret);
  }
  m31865_start(&s_m31865_1, SPI2_HOST, MAX31865_4WIRE, PIN_TEMP1_CS, 1, MAX31865_BUS_FREQUENCY);
  // m31865_start(&s_m31865_2, SPI2_HOST, PIN_TEMP2_CS, 1, MAX31865_BUS_FREQUENCY);
  xTaskCreate(temp_sensor_proc, TAG, 2048, nullptr, tskIDLE_PRIORITY + 5, &s_task_handle);
}
