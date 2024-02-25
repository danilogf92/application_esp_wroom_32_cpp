#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>

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

enum class NetworkType
{
  AP,
  STA,
};

class Network
{
  protected:
  static void handle_wifi_event (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

  private:
  std::string wifi_ssid;
  std::string wifi_password;
  uint16_t timeout;
  int8_t clients_connected;
  int8_t disconnection_err_count;
  esp_netif_t* esp_netif;
  void wifi_connect_ap (void);
  esp_err_t wifi_connect_sta (void);
  Network (std::string _wifi_ssid, std::string _wifi_password, uint16_t timeout = 35000);

  friend class NetworkFactory;

  public:
  ~Network ();
  void print_credentials (void);
  void stop (void);
  void start (NetworkType type);
  int8_t get_clients_ap (void);
};
class NetworkFactory
{
  public:
  static Network* create_network (std::string _wifi_ssid, std::string _wifi_password, uint16_t timeout = 35000)
  {
    return new Network (_wifi_ssid, _wifi_password, timeout);
  }
};
#endif