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
  AP_STA
};

class Network
{
  protected:
  static void handle_wifi_event (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

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
  bool loop_active;

  // void start_ap_sta (void);
  // Network (std::string _ssid, std::string _password, uint16_t timeout = 35000);
  Network (std::string _ap_ssid, std::string _ap_password, std::string _sta_ssid, std::string _sta_password, uint16_t timeout = 35000);

  friend class NetworkFactory;

  public:
  ~Network ();
  void print_credentials (void);
  // void stop (void);
  // void start (void);
  void network_connection_type (NetworkType type);
  int8_t get_clients_ap (void);
  void wifi_stop_sta ();
  void wifi_init_ap (void);
  esp_err_t wifi_init_sta (void);
  void wifi_stop_ap ();

};
class NetworkFactory
{
  public:
  // static Network* create_network_ap (std::string _ssid, std::string _password, uint16_t timeout = 35000)
  // {
  //   return new Network (_ssid, _password, timeout);
  // }

  // static Network* create_network_sta (std::string _ssid, std::string _password, uint16_t timeout = 35000)
  // {
  //   return new Network (_ssid, _password, timeout);
  // }

  static Network* create_network_ap_sta (std::string ap_ssid, std::string ap_password, std::string sta_ssid, std::string sta_password, uint16_t timeout = 35000)
  {
    return new Network (ap_ssid, ap_password, sta_ssid, sta_password, timeout);
  }
};
#endif