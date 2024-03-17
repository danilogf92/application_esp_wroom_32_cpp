#ifndef IGPIO_HPP
#define IGPIO_HPP

#include <stdio.h>
#include <string>
#include "driver/gpio.h"

class IGpio
{
  public:
  virtual bool get_status_pin () = 0;
  virtual gpio_num_t get_pin () const = 0;
  virtual std::string get_name (void) const = 0;
  virtual ~IGpio () = default;
};

#endif /* IGPIO_HPP */
