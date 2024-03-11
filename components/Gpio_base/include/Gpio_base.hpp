#ifndef GPIO_BASE_HPP
#define GPIO_BASE_HPP

#include <string>
#include "driver/gpio.h"
#include "debug_def.h"

class Gpio_base
{
  protected:
  gpio_num_t pin;
  std::string name;

  public:
  ~Gpio_base ();
  Gpio_base (gpio_num_t _pin, std::string _name, gpio_mode_t _mode);
};

#endif /* GPIO_BASE_HPP */
