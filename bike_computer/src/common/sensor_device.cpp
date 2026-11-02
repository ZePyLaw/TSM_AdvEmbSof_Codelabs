/**
 ******************************************************************************
 * @file        : sensor_device.cpp
 * @brief       : sensor_device class
 * @author      : Jean Vallet <jean.vallet@hes-so.ch>
 * @date        : 13-10-2026
 ******************************************************************************
 * @copyright   : Copyright (c) 2026
 *                HES-SO Master of Science in Engineering
 * @attention   : SPDX-License-Identifier: MIT OR Apache-2.0
 ******************************************************************************
 * @details
 * Implementation of the sensor_device class.
 ******************************************************************************
 */

#include "sensor_device.hpp"

// zephyr
#if !CONFIG_EMUL
#include <zephyr/drivers/sensor.h>
#endif  // !CONFIG_EMUL

// zpp-lib
#include "zpp_include/zpp_log.hpp"

ZPP_LOG_MODULE_REGISTER(sensor_bm280, CONFIG_APP_LOG_LEVEL);

namespace bike_computer {

zpp_lib::ZephyrResult SensorDevice::initialize() {
  // Create a ZephyrResult object
  zpp_lib::ZephyrResult result;

#if !CONFIG_EMUL
  // Retrieve the sensor device from the devicetree
  // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
  _sensor_device = DEVICE_DT_GET(DT_INST(0, bosch_bme280));

  // Check wether the sensor device is ready
  const bool device_ready = device_is_ready(_sensor_device);
  if (!device_ready) {
    result.assign_error(zpp_lib::ZephyrErrorCode::Nxio);
  }
#else
  // No hardware initialization is required for QEMU
#endif  // !CONFIG_EMUL

  return result;
}

zpp_lib::ZephyrResult SensorDevice::read_temperature(float& temperature) {
  // Create a ZephyrResult object
  zpp_lib::ZephyrResult result;

#if !CONFIG_EMUL
  // Create a variable to hold the sensor reading
  struct sensor_value sensor_value{};

  // Fetch data from the sensor
  const int fetch_result = sensor_sample_fetch(_sensor_device);
  if (fetch_result < 0) {
    result.assign_error(zpp_lib::ZephyrErrorCode::Io);
    return result;
  }

  // Retrive the temperature from the sensor
  const int channel_result = sensor_channel_get(_sensor_device, SENSOR_CHAN_AMBIENT_TEMP, &sensor_value);
  if (channel_result < 0) {
    result.assign_error(zpp_lib::ZephyrErrorCode::Io);
    return result;
  }

  // Log the temperature reading
  ZPP_LOG_INF("T=%.2f [deg C]", sensor_value_to_double(&sensor_value));

  // Convert the sensor reading to a float
  temperature = sensor_value_to_float(&sensor_value);
#else
  // Emulated sensor value
  temperature = 25.0F;
  ZPP_LOG_INF("T=%.2f [deg C]", static_cast<double>(temperature));
#endif  // !CONFIG_EMUL

  return result;
}

zpp_lib::ZephyrResult SensorDevice::read_humidity(float& humidity) {
  // Create a ZephyrResult object
  zpp_lib::ZephyrResult result;

#if !CONFIG_EMUL
  // Create a variable for the sensor value
  struct sensor_value sensor_value{};

  // Fetch data from the sensor
  const int fetch_result = sensor_sample_fetch(_sensor_device);
  if (fetch_result < 0) {
    result.assign_error(zpp_lib::ZephyrErrorCode::Io);
    return result;
  }

  // Retrive the humidity from the sensor
  const int channel_result = sensor_channel_get(_sensor_device, SENSOR_CHAN_HUMIDITY, &sensor_value);
  if (channel_result < 0) {
    result.assign_error(zpp_lib::ZephyrErrorCode::Io);
    return result;
  }

  // Log the temperature reading
  ZPP_LOG_INF("H=%.1f [%%]", sensor_value_to_double(&sensor_value));

  // Convert the sensor reading to a float
  humidity = sensor_value_to_float(&sensor_value);
#else
  // Emulated sensor value
  humidity = 50.0F;
  ZPP_LOG_INF("H=%.1f [%%]", static_cast<double>(humidity));
#endif  // !CONFIG_EMUL

  return result;
}

}  // namespace bike_computer
