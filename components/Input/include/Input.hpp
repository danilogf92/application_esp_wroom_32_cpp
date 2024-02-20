#ifndef INPUT_HPP
#define INPUT_HPP

#include <string>
#include "driver/gpio.h"
#include "IInput.hpp"

class Input : public IInput
{
  private:
  gpio_num_t pin;
  std::string status;
  std::string name;

  public:
  Input ();
  Input (gpio_num_t _pin, std::string _name);
  bool get_status_pin () const override;
  std::string get_name (void) const override;
  gpio_num_t get_pin () override;
};

#endif /* SENSOR_HPP */
