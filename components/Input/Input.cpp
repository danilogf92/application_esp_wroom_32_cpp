#include <iostream>
#include <cstdlib>
#include <cstring>

#include "Input.hpp"
#include "debug.h"

// #define INPUT_DEBUG

Input::Input () : pin (GPIO_NUM_NC), status (""), name ("")
{
#ifdef INPUT_DEBUG
  debug_warning ("Sensor: Created");
#endif
}

Input::Input (gpio_num_t _pin, std::string _name) : pin (_pin), status (""), name (_name)
{
  gpio_set_direction (pin, GPIO_MODE_INPUT);

#ifdef INPUT_DEBUG
  debug_warning ("Input name \"%s\" pin number: \"%d\" Initialized", name.c_str (), pin);
#endif
}

gpio_num_t Input::get_pin ()
{
#ifdef INPUT_DEBUG
  debug_warning ("Input pin is : %d", pin);
#endif
  return pin;
}

bool Input::get_status_pin () const
{
  bool status_local = gpio_get_level (pin);

#ifdef INPUT_DEBUG
  debug_warning ("pin %d =  %s", pin, debug_get_bool_status (status_local));
#endif

  return status_local;
}


std::string Input::get_name (void) const
{
#ifdef INPUT_DEBUG
  debug_warning ("Input name \"%s\"", name.c_str ());
#endif
  return name;
}
