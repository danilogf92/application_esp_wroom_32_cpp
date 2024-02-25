#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "ISensor.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum class SensorFilterType
{
  NONE,
  MEDIAN,
  KALMAN
};
class Sensor : public ISensor
{
  private:
  float value;
  uint16_t sampling_period;
  bool enable_sensor;
  std::string name;
  float kalman_value;
  float median_value;
  std::vector<float> filter;
  static void sensor_task (void* arg);
  TaskHandle_t task_handle;
  std::function<float ()> fn;
  void init (void) override;
  float kalman_gain;
  float current_estimate;
  float err_estimate;
  float last_estimate;
  float q;
  float err_measure;
  Sensor (std::string _name, callback_config_function callback_config, std::function<float ()> func, uint16_t samples);

  friend class SensorFactory;

  public:
  ~Sensor ();
  float get_value () override;
  void update_value (float new_value) override;
  float get_median_value () override;
  float get_kalman_value () override;
  std::string get_name () const override;
  esp_err_t enable (void) override;
  esp_err_t disable (void) override;
  uint16_t get_sample_period ();
};

class SensorFactory
{
  public:
  static Sensor* create_sensor (std::string _name, callback_config_function callback_config, std::function<float ()> func, uint16_t samples)
  {
    return new Sensor (_name, callback_config, func, samples);
  }
};

#endif /* SENSOR_HPP */
