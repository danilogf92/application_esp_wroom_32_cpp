#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "ISensor.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
class Sensor : public ISensor
{
  private:
  float value;
  uint16_t sampling_period;
  bool enable_sensor;
  std::string name;
  float filtered_value;
  std::vector<float> filter;
  static void sensor_task (void* arg);
  TaskHandle_t task_handle;
  std::function<float ()> fn;
  void init (void) override;

  public:
  Sensor (std::string _name, callback_config_function callback_config, std::function<float ()> func, uint16_t samples);
  ~Sensor ();
  float get_value () override;
  void update_value (float new_value) override;
  float get_filtered_value () override;
  std::string get_name () const override;
  esp_err_t enable (void) override;
  esp_err_t disable (void) override;
  uint16_t get_sample_period ();
};

#endif /* SENSOR_HPP */
