#include <type_traits>
#include <string.h>
#include <esp_err.h>
#include <esp_log.h>
#include <math.h>
#include <hal/spi_types.h>
#include <driver/spi_master.h>
#include "m31865.h"
#include <freertos/task.h>

const char *const TAG = "MAX31865";

template<typename T>
T read_register(spi_device_handle_t handle, uint8_t addr, bool keep_cs_active);
template<typename T>
void write_register(spi_device_handle_t handle, uint8_t addr, T data, bool keep_cs_active);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
esp_err_t m31865_start(
  spi_device_handle_t* handle,
  spi_host_device_t spi_peripheral,
  m31865_connector_type wires,
  int cs,
  uint8_t mode,
  uint32_t freq
)
{
  spi_device_interface_config_t devcfg =
  {
    .address_bits = 8,
    .mode = mode,
    .clock_speed_hz = static_cast<int>(freq),
    .spics_io_num = cs,
    .queue_size = 7,
    .pre_cb = NULL,
  };
  esp_err_t ret=spi_bus_add_device(spi_peripheral, &devcfg, handle);
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to initialize spi interface, spi_bus_add_device %d", ret);
    return ret;
  }

  spi_device_acquire_bus(*handle, portMAX_DELAY);
  m31865_set_wire(*handle, wires);
  m31865_enable_bias(*handle, false);
  m31865_enable_auto_convert(*handle, false);
  m31865_set_thresholds(*handle, 0, 0xFFFF);
  m31865_clear_fault(*handle);
  spi_device_release_bus(*handle);

  return ESP_OK;
}

void m31865_set_wire(spi_device_handle_t handle, m31865_connector_type connector_type)
{
  uint8_t t = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
  if (connector_type == MAX31865_3WIRE) {
    t |= MAX31865_CONFIG_3WIRE;
  } else {
    // 2 or 4 wire
    t &= ~MAX31865_CONFIG_3WIRE;
  }
  write_register(handle, MAX31865_CONFIG_REG, t, false);
}

void m31865_enable_auto_convert(spi_device_handle_t handle, bool b)
{
  uint8_t t = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
  if (b) {
    t |= MAX31865_CONFIG_MODEAUTO;
  } else {
    t &= ~MAX31865_CONFIG_MODEAUTO;
  }
  write_register(handle, MAX31865_CONFIG_REG, t, false);
}

void m31865_enable_50Hz(spi_device_handle_t handle, bool b)
{
  uint8_t t = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
  if (b) {
    t |= MAX31865_CONFIG_FILT50HZ;
  } else {
    t &= ~MAX31865_CONFIG_FILT50HZ;
  }
  write_register(handle, MAX31865_CONFIG_REG, &t, false);
}

void m31865_enable_bias(spi_device_handle_t handle, bool b)
{
  uint8_t t = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
  if (b) {
    t |= MAX31865_CONFIG_BIAS;
  } else {
    t &= ~MAX31865_CONFIG_BIAS;
  }
  write_register(handle, MAX31865_CONFIG_REG, t, false);
}

void m31865_set_thresholds(spi_device_handle_t handle, uint16_t lower, uint16_t upper)
{
  write_register<uint8_t>(handle, MAX31865_LFAULTLSB_REG, lower & 0xFF, true);
  write_register<uint8_t>(handle, MAX31865_LFAULTMSB_REG, lower >> 8, true);
  write_register<uint8_t>(handle, MAX31865_HFAULTLSB_REG, upper & 0xFF, true);
  write_register<uint8_t>(handle, MAX31865_HFAULTMSB_REG, upper >> 8, false);
}

uint16_t m31865_get_lower_threshold(spi_device_handle_t handle)
{
  return read_register<uint16_t>(handle, MAX31865_LFAULTMSB_REG, false);
}

uint16_t m31865_get_upper_threshold(spi_device_handle_t handle)
{
  return read_register<uint16_t>(handle, MAX31865_HFAULTMSB_REG, false);
}

uint8_t m31865_read_fault(spi_device_handle_t handle, m31865_fault_cycle fault_cycle)
{
  if (fault_cycle)
  {
    uint8_t cfg_reg = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
    cfg_reg &= 0x11; // mask out wire and filter bits
    switch (fault_cycle) {
    case MAX31865_FAULT_AUTO:
      write_register<uint8_t>(handle, MAX31865_CONFIG_REG, cfg_reg | 0b10000100, true);
      vTaskDelay(1 / portTICK_PERIOD_MS);
      break;
    case MAX31865_FAULT_MANUAL_RUN:
      write_register<uint8_t>(handle, MAX31865_CONFIG_REG, cfg_reg | 0b10001000, true);
      return 0;
    case MAX31865_FAULT_MANUAL_FINISH:
      write_register<uint8_t>(handle, MAX31865_CONFIG_REG, cfg_reg | 0b10001100, true);
      return 0;
    case MAX31865_FAULT_NONE:
    default:
      break;
    }
  }
  uint8_t fault = read_register<uint8_t>(handle, MAX31865_FAULTSTAT_REG, false);
  return fault;
}

void m31865_clear_fault(spi_device_handle_t handle)
{
  uint8_t t = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
  t &= ~0x2C;
  t |= MAX31865_CONFIG_FAULTSTAT;
  write_register(handle, MAX31865_CONFIG_REG, t, false);
}

uint16_t m31865_read_rtd(spi_device_handle_t handle)
{
  m31865_clear_fault(handle);
  m31865_enable_bias(handle, true);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  uint8_t t = read_register<uint8_t>(handle, MAX31865_CONFIG_REG, true);
  t |= MAX31865_CONFIG_1SHOT;
  write_register(handle, MAX31865_CONFIG_REG, t, true);
  vTaskDelay(65 / portTICK_PERIOD_MS);

  uint16_t rtd = read_register<uint16_t>(handle, MAX31865_RTDMSB_REG, true);

  m31865_enable_bias(handle, false); // Disable bias current again to reduce selfheating.

  // remove fault
  rtd >>= 1;

  return rtd;
}

template<typename T>
T read_register(spi_device_handle_t handle, uint8_t addr, bool keep_cs_active)
{
  addr &= 0x7F; // make sure top bit is not set

  assert((std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>));
  spi_transaction_t t = {
    .flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_VARIABLE_ADDR,
    .addr = addr,
    .length = sizeof(T),
    .rxlength = sizeof(T),
    .rx_data = {}
  };
  if (keep_cs_active)
    t.flags |= SPI_TRANS_CS_KEEP_ACTIVE;
  esp_err_t ret = spi_device_polling_transmit(handle, &t);
  assert(ret == ESP_OK);
  return *(reinterpret_cast<T*>(t.rx_data));
}

template<typename T>
void write_register(spi_device_handle_t handle, uint8_t addr, T data, bool keep_cs_active) {
  addr |= 0x80; // make sure top bit is set

  assert((std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>));
  spi_transaction_t t = {
    .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_VARIABLE_ADDR,
    .addr = addr,
    .length = sizeof(T),
  };
  *((T*)t.tx_data) = data;
  if (keep_cs_active)
    t.flags |= SPI_TRANS_CS_KEEP_ACTIVE;
  esp_err_t ret = spi_device_polling_transmit(handle, &t);
  assert(ret == ESP_OK);
}
#pragma GCC diagnostic pop

constexpr float RTD_A = 3.9083e-3;
constexpr float RTD_B = -5.775e-7;
constexpr float RTD_C = -4.183e-12;
constexpr float A[6] = {-242.02, 2.2228, 2.5859e-3, 4.8260e-6, 2.8183e-8, 1.5243e-10};

float calculate_temperature(uint16_t rtd, float rtd_nominal, float ref_resistor) {
  float Rrtd = (rtd * ref_resistor) / (1U << 15U);

  float Z1, Z2, Z3, Z4, temperature;
  Z1 = -RTD_A;
  Z2 = RTD_A * RTD_A - (4 * RTD_B);
  Z3 = (4 * RTD_B) / rtd_nominal;
  Z4 = 2 * RTD_B;
  temperature = Z2 + (Z3 * Rrtd);
  temperature = (sqrt(temperature) + Z1) / Z4;

  if (temperature > 0.0) {
    return temperature;
  }

  Rrtd /= rtd_nominal;
  Rrtd *= 100.0;
  return A[0] + A[1] * Rrtd + A[2] * pow(Rrtd, 2) + A[3] * pow(Rrtd, 3) +
         A[4] * pow(Rrtd, 4) + A[5] * pow(Rrtd, 5);
}
