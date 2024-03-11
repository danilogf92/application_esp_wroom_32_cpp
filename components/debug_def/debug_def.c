#include <stdio.h>
#include <stdarg.h>
#include "debug_def.h"

const char* debug_get_bool_status (bool _status)
{
  return _status ? "true" : "false";
}
