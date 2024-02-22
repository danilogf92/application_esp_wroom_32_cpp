#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "driver/gpio.h"
// #include "esp_spiffs.h"
// #include "nvs_flash.h"
// #include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

  void init_end_points (void);

#ifdef __cplusplus
}
#endif

#endif
