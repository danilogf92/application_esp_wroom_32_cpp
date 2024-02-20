#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define COMMON_DATA_MINIMUM                 100
#define COMMON_DATA_MILISECOND              1000 
#define COMMON_DATA_SECOND                  1000 
#define COMMON_DATA_MINUTE                  60*1000 
#define COMMON_DATA_HOUR                    60*60*1000 

#ifdef __cplusplus
extern "C" {
#endif

#define DELAY_SECONDS(value) \
          vTaskDelay (pdMS_TO_TICKS (value))

#define ON true
#define OFF false

#ifdef __cplusplus
}
#endif


#endif