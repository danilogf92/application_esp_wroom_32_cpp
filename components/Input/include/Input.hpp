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
  Input (gpio_num_t _pin, std::string _name);

  friend class InputFactory;

  public:
  ~Input ();
  bool get_status_pin () const override;
  std::string get_name (void) const override;
  gpio_num_t get_pin () const override;
};

class InputFactory
{
  public:
  static Input* create_input (gpio_num_t pin_out, const std::string& name)
  {
    return new Input (pin_out, name);
  }
};

#endif /* INPUT_HPP */
