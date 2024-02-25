#include "Output.hpp"
#include "driver/gpio.h"
#include <iostream>
#include "esp_log.h"
#include "debug.h"

// #define OUTPUT_DEBUG

Output::~Output ()
{
#ifdef OUTPUT_DEBUG
  debug_warning ("Output: %s destructor", name.c_str ());
#endif
}

Output::Output (gpio_num_t pin, std::string _name) : pin_out (pin), status_pin (false), name (_name)
{
  // esp_log_level_set ("gpio", ESP_LOG_NONE);

  // gpio_config_t io_conf;
  // io_conf.intr_type = GPIO_INTR_DISABLE;
  // io_conf.mode = GPIO_MODE_OUTPUT;
  // io_conf.pin_bit_mask = ( 1ULL << pin );
  // io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  // gpio_config (&io_conf);

  gpio_set_direction (pin, GPIO_MODE_OUTPUT);

#ifdef OUTPUT_DEBUG
  debug_warning ("Output name \"%s\" pin number: \"%d\" Initialized", name.c_str (), pin);
#endif
}

bool Output::get_status_pin (void) const
{
  return status_pin;
}

void Output::set_pin (bool state)
{
  gpio_set_level (pin_out, ( uint32_t ) state);
  status_pin = state;

#ifdef OUTPUT_DEBUG
  debug_warning ("Set pin: \"%d\", state: \"%s\"", pin_out, debug_get_bool_status (( bool ) state));
#endif
}

std::string Output::get_name (void) const
{
  return name;
}
