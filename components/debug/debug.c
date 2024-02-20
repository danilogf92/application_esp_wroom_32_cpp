#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

const char* debug_get_bool_status (bool _status)
{
  return _status ? "true" : "false";
}
