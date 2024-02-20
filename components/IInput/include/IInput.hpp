#ifndef IINPUT_HPP
#define IINPUT_HPP

#include <stdio.h>
#include <string>
#include "driver/gpio.h"

class IInput
{
  public:
  virtual bool get_status_pin () const = 0;
  virtual gpio_num_t get_pin () = 0;
  virtual std::string get_name (void) const = 0;
  virtual ~IInput () = default;
};

#endif /* ISENSOR_HPP */
