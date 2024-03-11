#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <iostream>
#include <string>
#include "driver/gpio.h"
#include "IOutput.hpp"
#include "Gpio_base.hpp"

class Output : public Gpio_base, public IOutput
{
  private:
  bool status_pin;
  Output (gpio_num_t _pin, std::string _name);

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
  static Output* create_output (gpio_num_t _pin, const std::string& name)
  {
    return new Output (_pin, name);
  }
};

#endif /* OUTPUT_HPP */
