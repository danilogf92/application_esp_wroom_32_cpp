#include "Output.hpp"
#include "driver/gpio.h"
#include <iostream>
#include "debug.h"

// #define OUTPUT_DEBUG

Output::~Output ()
{
#ifdef OUTPUT_DEBUG
  debug_warning ("Output: %s destructor", name.c_str ());
#endif
}

Output::Output (gpio_num_t pin, std::string _name) : pin_out (pin), status ("Active"), name (_name)
{
  gpio_set_direction (pin, GPIO_MODE_INPUT_OUTPUT);
#ifdef OUTPUT_DEBUG
  debug_warning ("Output name \"%s\" pin number: \"%d\" Initialized", name.c_str (), pin);
#endif
}

bool Output::get_status_pin () const
{
  bool status_local = gpio_get_level (pin_out);
#ifdef OUTPUT_DEBUG
  debug_warning ("Status pin %d: %s", pin_out, debug_get_bool_status (status_local));
#endif
  return status_local;
}

void Output::set_pin (uint32_t state)
{
  gpio_set_level (pin_out, state);
#ifdef OUTPUT_DEBUG
  debug_warning ("Set pin: \"%d\", state: \"%s\"", pin_out, debug_get_bool_status (( bool ) state));
#endif
}

std::string Output::get_name (void) const
{
  return name;
}
