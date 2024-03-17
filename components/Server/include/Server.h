// #ifndef SERVER_HPP
// #define SERVER_HPP

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <time.h>

// #include "cJSON.h"
// #include "esp_http_server.h"
// #include "esp_log.h"
// #include "driver/gpio.h"
// typedef struct server
// {
//   gpio_num_t led;
//   float temperature;
//   float distance;
// }server_t;

// #ifdef __cplusplus
// extern "C" {
// #endif

//   void init_end_points (void);
//   extern server_t server_data;

// #ifdef __cplusplus
// }
// #endif

// #endif


#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif
  typedef struct
  {
    gpio_num_t led;
    float temperature;
    float distance;
  } server_data_t;

  void init_end_points (void);
  extern server_data_t server_data;

#ifdef __cplusplus
}
#endif

#endif /* SERVER_H */
