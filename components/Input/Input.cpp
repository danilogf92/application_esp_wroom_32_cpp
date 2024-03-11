#include "Input.hpp"

// #define INPUT_DEBUG

#define LONG_PRESS_TIME ((200)/portTICK_PERIOD_MS)

Input::Input (gpio_num_t _pin, std::string _name) : Gpio_base (_pin, _name, GPIO_MODE_INPUT), debounce_time { 0 }, state { InputStateMachine::NOT_PRESSED }, last_press_time { 0 }
{
#ifdef INPUT_DEBUG
  debug_warning ("Input name \"%s\" pin number: \"%d\" Constructor", name.c_str (), pin);
#endif
}

Input::Input (gpio_num_t _pin, std::string _name, uint16_t _debounce_time) : Gpio_base (_pin, _name, GPIO_MODE_INPUT), debounce_time { _debounce_time }, state { InputStateMachine::NOT_PRESSED }, last_press_time { 0 }
{
#ifdef INPUT_DEBUG
  debug_warning ("Input name \"%s\" pin number: \"%d\" debounce_time: \"%d\" Constructor", name.c_str (), pin, debounce_time);
#endif
}

Input::~Input ()
{
  Gpio_base::~Gpio_base ();
#ifdef INPUT_DEBUG
  debug_warning ("Input name \"%s\" pin number: \"%d\" Destructor", name.c_str (), pin);
#endif
}

gpio_num_t Input::get_pin () const
{
  return pin;
}

bool Input::get_status_pin ()
{
  if ( debounce_time > 0 )
  {
    this->input_state_machine ();
    switch ( state )
    {
      case InputStateMachine::PRESSED:
      case InputStateMachine::LONG_PRESSED:
        return true;

      default:
        return false;
    }
  }

  return this->_status_pin ();
}

bool Input::_status_pin ()
{
  return ( bool ) !gpio_get_level (pin);
}


std::string Input::get_name (void) const
{
  return name;
}

void Input::input_state_machine (void)
{
  if ( debounce_time > 0 )
  {
    switch ( state )
    {
      case InputStateMachine::NOT_PRESSED:
#ifdef INPUT_DEBUG
        debug_warning ("InputStateMachine::NOT_PRESSED");
#endif
        if ( _status_pin () )
        {
          state = InputStateMachine::DEBOUNCE;
          last_press_time = xTaskGetTickCount ();
        }
        break;

      case InputStateMachine::DEBOUNCE:
#ifdef INPUT_DEBUG
        debug_warning ("InputStateMachine::DEBOUNCE time_capture=%ld\tdebounce_normal=%ld\ttime_normal=%ld", debounce_time / portTICK_PERIOD_MS, last_press_time, xTaskGetTickCount ());
#endif
        if ( !_status_pin () )
        {
          state = InputStateMachine::NOT_PRESSED;
        }
        else if ( xTaskGetTickCount () - last_press_time >= ( debounce_time / portTICK_PERIOD_MS ) )
        {
          state = InputStateMachine::PRESSED;
          last_press_time = xTaskGetTickCount ();
        }
        break;

      case InputStateMachine::PRESSED:
#ifdef INPUT_DEBUG
        debug_warning ("InputStateMachine::PRESSED time_capture=%ld\ttime_normal=%ld", last_press_time, xTaskGetTickCount ());
#endif
        if ( !_status_pin () )
        {
          state = InputStateMachine::NOT_PRESSED;
        }
        else if ( xTaskGetTickCount () - last_press_time >= LONG_PRESS_TIME )
        {
          state = InputStateMachine::LONG_PRESSED;
        }
        break;

      case InputStateMachine::LONG_PRESSED:
#ifdef INPUT_DEBUG
        debug_warning ("InputStateMachine::LONG_PRESSED time=%ld", LONG_PRESS_TIME);
#endif
        if ( !_status_pin () )
        {
          state = InputStateMachine::NOT_PRESSED;
          last_press_time = 0;
        }
        break;

      default:
        state = InputStateMachine::NOT_PRESSED;
#ifdef INPUT_DEBUG
        debug_warning ("InputStateMachine::NOT_PRESSED");
#endif
        last_press_time = 0;
        break;
    }
  }

}