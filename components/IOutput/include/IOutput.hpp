#ifndef IOUTPUT_HPP
#define IOUTPUT_HPP

#include <cstdint>
#include <string>

class IOutput
{
  public:
  virtual bool get_status_pin () const = 0;
  virtual void set_pin (uint32_t state) = 0;
  virtual std::string get_name (void) const = 0;
  virtual ~IOutput () = default;
};

#endif /* IOUTPUT_HPP */
