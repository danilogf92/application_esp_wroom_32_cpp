#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <esp_err.h>

#include "Application.hpp"
#include "Output.hpp"
#include "Sensor.hpp"
#include "ultrasonic.h"
#include "debug.h"
#include "common_data.h"

extern "C" {
  void app_main (void);
}

void app_main (void)
{
  // Create application

  // Add a minimum delay of 100 ms to prevent the watchdog......
  while ( true )
  {
    DELAY (MINIMUM);
  }
}



