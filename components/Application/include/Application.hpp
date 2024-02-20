#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>
#include <vector>
#include "IOutput.hpp"
#include "ISensor.hpp"
#include "IInput.hpp"

class Application
{
  private:
  std::string device_name;
  std::string mac_address;
  std::vector<IOutput*> outputs;
  std::vector<ISensor*> sensors;
  std::vector<IInput*> inputs;

  public:
  Application (std::string _name);
  ~Application ();
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
  float get_sensor_data (std::string _name, bool filter = false);
  void remove_sensor (std::string _name);
};

#endif /* APPLICATION_HPP */
