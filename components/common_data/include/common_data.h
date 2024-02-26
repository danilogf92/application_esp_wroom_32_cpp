#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MINIMUM                 100
#define ONE_MILISECOND          1
#define ONE_SECOND              1000 
#define ONE_MINUTE              60*1000
#define ONE_HOUR                60*60*1000 

#ifdef __cplusplus
extern "C" {
#endif

#define DELAY(value) \
          vTaskDelay (pdMS_TO_TICKS (value))

#define ON true
#define OFF false

#ifdef __cplusplus
}
#endif


#endif