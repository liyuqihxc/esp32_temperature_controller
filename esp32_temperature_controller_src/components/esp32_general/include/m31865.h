#ifndef _M31865_H_
#define _M31865_H_

#define MAX31865_CONFIG_REG 0x00
#define MAX31865_CONFIG_BIAS 0x80
#define MAX31865_CONFIG_MODEAUTO 0x40
#define MAX31865_CONFIG_MODEOFF 0x00
#define MAX31865_CONFIG_1SHOT 0x20
#define MAX31865_CONFIG_3WIRE 0x10
#define MAX31865_CONFIG_24WIRE 0x00
#define MAX31865_CONFIG_FAULTSTAT 0x02
#define MAX31865_CONFIG_FILT50HZ 0x01
#define MAX31865_CONFIG_FILT60HZ 0x00

#define MAX31865_RTDMSB_REG 0x01
#define MAX31865_RTDLSB_REG 0x02
#define MAX31865_HFAULTMSB_REG 0x03
#define MAX31865_HFAULTLSB_REG 0x04
#define MAX31865_LFAULTMSB_REG 0x05
#define MAX31865_LFAULTLSB_REG 0x06
#define MAX31865_FAULTSTAT_REG 0x07

#define MAX31865_FAULT_HIGHTHRESH 0x80
#define MAX31865_FAULT_LOWTHRESH 0x40
#define MAX31865_FAULT_REFINLOW 0x20
#define MAX31865_FAULT_REFINHIGH 0x10
#define MAX31865_FAULT_RTDINLOW 0x08
#define MAX31865_FAULT_OVUV 0x04

const uint32_t MAX31865_BUS_FREQUENCY = 1 * 1000 *1000; // 1MHz, Max 5MHz

typedef enum
{
  MAX31865_2WIRE = 0,
  MAX31865_3WIRE = 1,
  MAX31865_4WIRE = 0
}m31865_connector_type;

typedef enum {
  MAX31865_FAULT_NONE = 0,
  MAX31865_FAULT_AUTO,
  MAX31865_FAULT_MANUAL_RUN,
  MAX31865_FAULT_MANUAL_FINISH
} m31865_fault_cycle;

extern "C" esp_err_t m31865_start(
  spi_device_handle_t* handle,
  spi_host_device_t spi_peripheral,
  m31865_connector_type wires,
  int cs,
  uint8_t mode = 1,
  uint32_t freq = MAX31865_BUS_FREQUENCY
);
extern "C" void m31865_set_wire(spi_device_handle_t handle, m31865_connector_type connector_type);
extern "C" void m31865_enable_auto_convert(spi_device_handle_t handle, bool b);
extern "C" void m31865_enable_50Hz(spi_device_handle_t handle, bool b);
extern "C" void m31865_enable_bias(spi_device_handle_t handle, bool b);
extern "C" void m31865_set_thresholds(spi_device_handle_t handle, uint16_t lower, uint16_t upper);
extern "C" uint16_t m31865_get_lower_threshold(spi_device_handle_t handle);
extern "C" uint16_t m31865_get_upper_threshold(spi_device_handle_t handle);
extern "C" uint8_t m31865_read_fault(spi_device_handle_t handle, m31865_fault_cycle fault_cycle = MAX31865_FAULT_AUTO);
extern "C" void m31865_clear_fault(spi_device_handle_t handle);
extern "C" uint16_t m31865_read_rtd(spi_device_handle_t handle);



extern "C" float calculate_temperature(uint16_t rtd, float rtd_nominal, float ref_resistor);

#endif
