#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <mutex>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif.h"
#include "lwip/inet.h"

#include "debug_def.h"

enum class NetworkType
{
  AP,
  STA,
  AP_STA,
  NONE
};

class Network
{
  protected:
  static void handle_wifi_event (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
  static Network* network_instance;

  private:
  std::string ap_ssid;
  std::string ap_password;
  std::string sta_ssid;
  std::string sta_password;
  uint16_t timeout;
  int8_t clients_connected;
  int8_t disconnection_err_count;
  NetworkType network_type;
  esp_netif_t* ap_netif;
  esp_netif_t* sta_netif;
  Network (std::string _ap_ssid, std::string _ap_password, std::string _sta_ssid, std::string _sta_password, uint16_t _timeout);

  public:
  ~Network ();
  static Network* get_instance (std::string ap_ssid, std::string ap_password, std::string sta_ssid, std::string sta_password);
  void network_credentials (void);
  void network_connection_manager (NetworkType type);
  int8_t network_get_clients_connected (void);
  void network_init_ap (void);
  esp_err_t network_init_sta (void);
  void network_stop (void);
  void network_init_apsta (void);
};
#endif