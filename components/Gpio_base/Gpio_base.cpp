#include "Gpio_base.hpp"

// #define GPIO_BASE_DEBUG

Gpio_base::Gpio_base (gpio_num_t _pin, std::string _name, gpio_mode_t _mode) : pin (_pin), name (_name)
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = _mode;
  io_conf.pin_bit_mask = ( 1ULL << _pin );
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config (&io_conf);

#ifdef GPIO_BASE_DEBUG
  debug_warning ("Gpio_base name \"%s\" pin number: \"%d\" CONSTRUCTOR", name.c_str (), pin);
#endif
}

Gpio_base::~Gpio_base ()
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_DISABLE;
  io_conf.pin_bit_mask = ( 1ULL << pin );
  gpio_config (&io_conf);

#ifdef GPIO_BASE_DEBUG
  debug_warning ("Gpio_base name \"%s\" pin number: \"%d\" DESTRUCTOR", name.c_str (), pin);
#endif
}