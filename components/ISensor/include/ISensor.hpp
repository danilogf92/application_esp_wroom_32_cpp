#ifndef ISENSOR_HPP
#define ISENSOR_HPP

#include <string>
#include <vector>
#include "esp_err.h"

using callback_config_function = esp_err_t (*)( );

class ISensor
{
  private:
  virtual void update_value (float new_value) = 0;

  public:
  virtual void init (void) = 0;
  virtual esp_err_t enable (void) = 0;
  virtual esp_err_t disable (void) = 0;
  virtual std::string get_name () const = 0;
  virtual float get_value () = 0;
  virtual float get_filtered_value () = 0;
  virtual ~ISensor () = default;
};

#endif /* ISENSOR_HPP */



