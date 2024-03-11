#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <cstring> 
#include <string>
#include <vector>
#include <memory>

#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_err.h"
#include "debug_def.h"
#include "esp_mac.h"

#include "IOutput.hpp"
#include "ISensor.hpp"
#include "Sensor.hpp"
#include "IInput.hpp"
#include "Network.hpp"

class Application
{
  private:
  std::string device_name;
  std::string mac_address;
  std::vector<std::unique_ptr<IOutput>> outputs;
  std::vector<std::unique_ptr<IInput>> inputs;
  std::vector<std::unique_ptr<ISensor>> sensors;
  std::unique_ptr<Network> network;
  bool network_active;

  protected:
  Application (std::string _name);
  static Application* application_instance;

  public:
  ~Application ();
  static Application* get_instance (const std::string& value);
  Application (Application& other) = delete;
  void operator=(const Application&) = delete;

  std::string get_device_name () const;
  void print_mac_address (void);
  void print_device_details (void);
  void add_output (IOutput* output);
  void set_output (std::string _name, bool _state);
  void remove_output (std::string _name);
  void add_input (IInput* input);
  bool get_input_status (std::string _name);
  void remove_input (std::string _name);
  void add_sensor (ISensor* sensor);
  float get_sensor_data (std::string _name, SensorFilterType filter_type = SensorFilterType::NONE);
  void remove_sensor (std::string _name);
  void add_network (const Network* _network);
  void start_network (NetworkType _type);
  void stop_network (void);
  esp_err_t network_exist (void);
  int8_t get_clients_connected (void);
};

#endif /* APPLICATION_HPP */
