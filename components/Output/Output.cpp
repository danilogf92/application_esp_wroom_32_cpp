#include "Output.hpp"

// #define OUTPUT_DEBUG

Output::~Output ()
{
#ifdef OUTPUT_DEBUG
  debug_warning ("Output: name \"%s\" pin number: \"%d\" DESTRUCTOR", name.c_str (), pin);
#endif
}

Output::Output (gpio_num_t _pin, std::string _name) : Gpio_base (_pin, _name, GPIO_MODE_OUTPUT)
{
#ifdef OUTPUT_DEBUG
  debug_warning ("Output name \"%s\" pin number: \"%d\" CONSTRUCTOR", name.c_str (), pin);
#endif
}

bool Output::get_status_pin (void) const
{
  return status_pin;
}

void Output::set_pin (bool state)
{
  gpio_set_level (pin, ( uint32_t ) state);
  status_pin = state;

#ifdef OUTPUT_DEBUG
  debug_warning ("Set pin: \"%d\", state: \"%s\"", pin, debug_get_bool_status (( bool ) state));
#endif
}

std::string Output::get_name (void) const
{
  return name;
}
