#include <iostream>
#include <cstdlib>
#include <cstring>

#include "Input.hpp"
#include "debug.h"

// #define INPUT_DEBUG

Input::~Input ()
{
#ifdef INPUT_DEBUG
  debug_warning ("Input: %s destructor", name.c_str ());
#endif
}

Input::Input (gpio_num_t _pin, std::string _name) : pin (_pin), status (""), name (_name)
{
  // gpio_config_t io_conf;
  // io_conf.intr_type = GPIO_INTR_DISABLE;
  // io_conf.mode = GPIO_MODE_INPUT;
  // io_conf.pin_bit_mask = ( 1ULL << _pin );
  // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  // gpio_config (&io_conf);

  gpio_set_direction (pin, GPIO_MODE_INPUT);

#ifdef INPUT_DEBUG
  debug_warning ("Input name \"%s\" pin number: \"%d\" Initialized", name.c_str (), pin);
#endif
}

gpio_num_t Input::get_pin () const
{
  return pin;
}

bool Input::get_status_pin () const
{
  return ( bool ) gpio_get_level (pin);
}


std::string Input::get_name (void) const
{
  return name;
}
