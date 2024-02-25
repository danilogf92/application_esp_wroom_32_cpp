#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include "IOutput.hpp"
#include "driver/gpio.h"
#include <string>

class Output : public IOutput
{
  private:
  gpio_num_t pin_out;
  bool status_pin;
  std::string name;
  Output (gpio_num_t pin_out, std::string _name);

  friend class OutputFactory;

  public:
  ~Output ();
  bool get_status_pin (void) const override;
  void set_pin (bool state) override;
  std::string get_name (void) const override;
};

class OutputFactory
{
  public:
  static Output* create_output (gpio_num_t pin_out, const std::string& name)
  {
    return new Output (pin_out, name);
  }
};

#endif /* OUTPUT_HPP */
