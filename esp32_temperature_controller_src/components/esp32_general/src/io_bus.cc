#include <esp_err.h>
#include <esp_log.h>
#include <hal/spi_types.h>
#include <driver/spi_master.h>
#include "io_bus.h"
#include <string.h>

const char *const TAG = "io_bus";

esp_err_t spi_master_init(
  spi_host_device_t spi_peripheral,
  int miso, int mosi, int sclk
)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  spi_bus_config_t buscfg =
  {
    .mosi_io_num = mosi,
    .miso_io_num = miso,
    .sclk_io_num = sclk,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 0
  };
#pragma GCC diagnostic pop
  esp_err_t ret=spi_bus_initialize(spi_peripheral, &buscfg, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to initialize spi interface, spi_bus_initialize %d", ret);
  }
  return ret;
}

esp_err_t spi_master_register_device(
  spi_device_handle_t* handle,
  spi_host_device_t spi_peripheral,
  int cs, spi_mode mode, uint32_t freq
)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  spi_device_interface_config_t devcfg =
  {
    .mode = mode,
    .clock_speed_hz = static_cast<int>(freq),
    .spics_io_num = cs,
    .queue_size = 7,
    .pre_cb = NULL,
  };
#pragma GCC diagnostic pop
  esp_err_t ret=spi_bus_add_device(spi_peripheral, &devcfg, handle);
  if (ret != ESP_OK)
    ESP_LOGE(TAG, "Failed to initialize spi interface, spi_bus_add_device %d", ret);
}

void spi_write_then_read(
  spi_device_handle_t handle,
  const uint8_t *write_buffer,
  size_t write_len,
  uint8_t *read_buffer,
  size_t read_len,
  bool keep_cs_active
)
{
  esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = write_len;
    t.tx_buffer = write_buffer;
    t.user=nullptr;
    t.flags = SPI_TRANS_CS_KEEP_ACTIVE; // Keep CS active after data transfer
    ret=spi_device_polling_transmit(handle, &t);
    assert(ret==ESP_OK);
}
