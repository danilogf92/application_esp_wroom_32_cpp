#ifndef INPUT_HPP
#define INPUT_HPP

#include <string>
#include "driver/gpio.h"
#include "IInput.hpp"
#include "Gpio_base.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

enum class InputStateMachine
{
  NOT_PRESSED,
  DEBOUNCE,
  PRESSED,
  LONG_PRESSED
};

enum class InputEvents
{
  BUTTON_ON,
  BUTTON_OFF,
  TIMEOUT,
};

class Input : public Gpio_base, public IInput
{
  private:
  uint16_t debounce_time;
  InputStateMachine state;
  uint32_t last_press_time;
  void input_state_machine (void);
  bool _status_pin ();
  Input (gpio_num_t _pin, std::string _name);
  Input (gpio_num_t _pin, std::string _name, uint16_t _debounce_time);

  friend class InputFactory;

  public:
  ~Input ();
  bool get_status_pin () override;
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

  static Input* create_input (gpio_num_t pin_out, const std::string& name, uint8_t debounce_time)
  {
    return new Input (pin_out, name, debounce_time);
  }
};

#endif /* INPUT_HPP */
