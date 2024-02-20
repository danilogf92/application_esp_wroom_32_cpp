#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include "IOutput.hpp"
#include "driver/gpio.h"
#include <string>

class Output : public IOutput
{
  private:
  gpio_num_t pin_out;
  std::string status;
  std::string name;
  float filtered_value;

  public:
  ~Output ();
  Output (gpio_num_t pin_out, std::string _name);
  bool get_status_pin () const override;
  void set_pin (uint32_t state) override;
  std::string get_name (void) const override;
};

#endif /* OUTPUT_HPP */
