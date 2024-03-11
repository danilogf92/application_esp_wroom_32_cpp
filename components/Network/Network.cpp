#include "Network.hpp"

#define NETWORK_DEBUG

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

constexpr uint8_t MAX_ATTEMMPS = 5;
std::mutex network_mutex;
Network* Network::network_instance = nullptr;

static bool attempt_reconnect = false;
static EventGroupHandle_t wifi_events;
static int CONNECTED = BIT0;
static int DISCONNECTED = BIT1;

static const char* get_wifi_disconnection_string (wifi_err_reason_t wifi_err_reason);

Network::Network (std::string _ap_ssid, std::string _ap_password, std::string _sta_ssid, std::string _sta_password, uint16_t _timeout) :
  ap_ssid { _ap_ssid }, ap_password { _ap_password }, sta_ssid { _sta_ssid }, sta_password { _sta_password }, timeout { _timeout }
{
  clients_connected = 0;
  disconnection_err_count = 0;
  network_type = NetworkType::NONE;
  ap_netif = nullptr;
  sta_netif = nullptr;
  esp_err_t err = nvs_flash_init ();
  if ( err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND )
  {
    nvs_flash_erase ();
    nvs_flash_init ();
  }

  ESP_ERROR_CHECK (esp_netif_init ());

  ESP_ERROR_CHECK (esp_event_loop_create_default ());

  sta_netif = esp_netif_create_default_wifi_sta ();

  if ( nullptr == sta_netif )
  {
    debug_error ("Error: \"Create default WIFI STA\"");
  }

  ap_netif = esp_netif_create_default_wifi_ap ();
  if ( nullptr == ap_netif )
  {
    debug_error ("Error: \"Create default WIFI AP\"");
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT ();
  ESP_ERROR_CHECK (esp_wifi_init (&cfg));

  ESP_ERROR_CHECK (esp_event_handler_register (WIFI_EVENT, ESP_EVENT_ANY_ID, handle_wifi_event, this));
  ESP_ERROR_CHECK (esp_event_handler_register (IP_EVENT, IP_EVENT_STA_GOT_IP, handle_wifi_event, this));

  // ESP_ERROR_CHECK (esp_wifi_set_storage (WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_NULL));
  ESP_ERROR_CHECK (esp_wifi_start ());

#ifdef NETWORK_DEBUG
  debug_normal ("Network CONSTRUCTOR");
#endif 
}

Network* Network::get_instance (std::string ap_ssid, std::string ap_password, std::string sta_ssid, std::string sta_password)
{
  if ( nullptr == network_instance )
  {
    network_instance = new Network (ap_ssid, ap_password, sta_ssid, sta_password, 35000);
  }
  return network_instance;
}

Network::~Network ()
{
#ifdef NETWORK_DEBUG
  debug_error ("Network DESTRUCTOR");
#endif 
  esp_err_t err = nvs_flash_deinit ();
  if ( err != ESP_OK )
  {
#ifdef NETWORK_DEBUG
    debug_error ("nvs_flash_deinit");
#endif 
  }
}

void Network::network_credentials (void)
{
  debug_normal ("Network: ssid=%s, password=%s", ap_ssid.c_str (), ap_password.c_str ());
}

int8_t Network::network_get_clients_connected (void)
{
  if ( NetworkType::AP == network_type || NetworkType::AP_STA == network_type )
  {
    return clients_connected;
  }

#ifdef NETWORK_DEBUG
  debug_normal ("NetworkType::STA, Clients = -1");
#endif 

  return -1;
}

void Network::handle_wifi_event (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
#ifdef NETWORK_DEBUG
  debug_normal ("handle_wifi_event");
#endif 

  Network* obj_handle_wifi = reinterpret_cast< Network* >( arg );

#ifdef NETWORK_DEBUG
  debug_normal ("Attemmps = %d", obj_handle_wifi->disconnection_err_count);
#endif 

  switch ( event_id )
  {
    case WIFI_EVENT_STA_START:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_STA_START");
#endif 
      esp_wifi_connect ();
      break;

    case WIFI_EVENT_STA_CONNECTED:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_STA_CONNECTED");
#endif 
      obj_handle_wifi->disconnection_err_count = 0;
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      {
        wifi_event_sta_disconnected_t* wifi_event_sta_disconnected = ( wifi_event_sta_disconnected_t* ) event_data;
#ifdef NETWORK_DEBUG
        debug_warning ("DISCONNECTED %d: %s", wifi_event_sta_disconnected->reason, get_wifi_disconnection_string (( wifi_err_reason_t ) wifi_event_sta_disconnected->reason));
#endif 
        if ( attempt_reconnect )
        {
          if ( wifi_event_sta_disconnected->reason == WIFI_REASON_NO_AP_FOUND ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_ASSOC_LEAVE ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_AUTH_EXPIRE ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_AUTH_FAIL ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_CONNECTION_FAIL )
          {

            if ( obj_handle_wifi->disconnection_err_count++ < MAX_ATTEMMPS )
            {
              vTaskDelay (pdMS_TO_TICKS (5000));
              esp_wifi_connect ();
              break;
            }
            else
            {
#ifdef NETWORK_DEBUG
              debug_error ("WIFI retries exceeded");
#endif 
              esp_restart ();
            }
          }
        }
        xEventGroupSetBits (wifi_events, DISCONNECTED);
        break;
      }

    case IP_EVENT_STA_GOT_IP:
#ifdef NETWORK_DEBUG
      debug_normal ("IP_EVENT_STA_GOT_IP");
#endif 

      xEventGroupSetBits (wifi_events, CONNECTED);
      break;

    case WIFI_EVENT_SCAN_DONE:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_SCAN_DONE");
#endif 
      break;
    case WIFI_EVENT_STA_STOP:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_STA_STOP");
#endif 

      break;

    case WIFI_EVENT_AP_START:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_AP_START");
#endif 

      break;
    case WIFI_EVENT_AP_STOP:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_AP_STOP");
#endif 

      break;
    case WIFI_EVENT_AP_STACONNECTED:
      obj_handle_wifi->clients_connected++;
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_AP_STACONNECTED, Clients = %d", obj_handle_wifi->clients_connected);
#endif 

      break;
    case  WIFI_EVENT_AP_STADISCONNECTED:
      obj_handle_wifi->clients_connected--;
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_AP_STADISCONNECTED, Clients = %d", obj_handle_wifi->clients_connected);
#endif 

      break;
    case WIFI_EVENT_AP_PROBEREQRECVED:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI_EVENT_AP_PROBEREQRECVED");
#endif 
      break;

    default:
      break;
  }
}

void Network::network_init_ap (void)
{

#ifdef NETWORK_DEBUG
  debug_normal ("WIFI AP start");
#endif 

  if ( nullptr == ap_netif )
  {
#ifdef NETWORK_DEBUG
    debug_error ("Failed to create AP interface");
#endif  
    return;
  }
  ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_AP));
  wifi_config_t wifi_config = {};
  strcpy (reinterpret_cast< char* >( wifi_config.ap.ssid ), ap_ssid.c_str ());
  strcpy (reinterpret_cast< char* >( wifi_config.ap.password ), ap_password.c_str ());
  wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  wifi_config.ap.max_connection = 4;
  wifi_config.ap.beacon_interval = 100;
  wifi_config.ap.channel = 1;
  ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK (esp_wifi_start ());
}

esp_err_t Network::network_init_sta (void)
{
#ifdef NETWORK_DEBUG
  debug_normal ("WIFI STA start");
#endif  

  if ( nullptr == sta_netif )
  {
#ifdef NETWORK_DEBUG
    debug_error ("Failed to create STA interface");
#endif  
    return ESP_FAIL;
  }

  // esp_netif_dhcpc_stop (sta_netif);
  // esp_netif_ip_info_t ip_info;
  // IP4_ADDR (&ip_info.ip, 192, 168, 1, 170);
  // IP4_ADDR (&ip_info.gw, 192, 168, 1, 100);
  // IP4_ADDR (&ip_info.netmask, 255, 255, 255, 0);

  // esp_netif_set_ip_info (sta_netif, &ip_info);

  attempt_reconnect = true;
  wifi_events = xEventGroupCreate ();
  ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_STA));
  wifi_config_t wifi_config = {};
  strcpy (reinterpret_cast< char* >( wifi_config.sta.ssid ), sta_ssid.c_str ());
  strcpy (reinterpret_cast< char* >( wifi_config.sta.password ), sta_password.c_str ());
  ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK (esp_wifi_start ());
  EventBits_t result = xEventGroupWaitBits (wifi_events, CONNECTED | DISCONNECTED, true, false, pdMS_TO_TICKS (timeout));
  if ( result == CONNECTED )
  {
    return ESP_OK;
  }
  return ESP_FAIL;
}

void Network::network_init_apsta (void)
{
  wifi_config_t sta_config = {  };
  strcpy (reinterpret_cast< char* >( sta_config.sta.ssid ), sta_ssid.c_str ());
  strcpy (reinterpret_cast< char* >( sta_config.sta.password ), sta_password.c_str ());

  wifi_config_t ap_config = {  };
  strcpy (reinterpret_cast< char* >( ap_config.ap.ssid ), ap_ssid.c_str ());
  strcpy (reinterpret_cast< char* >( ap_config.ap.password ), ap_password.c_str ());
  ap_config.ap.channel = 1;
  ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  ap_config.ap.max_connection = 4;
  ap_config.ap.beacon_interval = 100;
  ap_config.ap.ssid_hidden = 0;

  attempt_reconnect = true;
  wifi_events = xEventGroupCreate ();
  ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_APSTA));
  network_mutex.lock ();
  debug_error ("esperar ap connect");
  ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_AP, &ap_config));
  network_mutex.unlock ();
  debug_error ("esperar sta connect");
  ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK (esp_wifi_start ());
}

void Network::network_connection_manager (NetworkType type)
{
#ifdef NETWORK_DEBUG
  debug_normal ("WIFI start");
#endif  
  network_type = type;
  switch ( type )
  {
    case NetworkType::AP:
      this->network_init_ap ();
      break;

    case NetworkType::STA:
      this->network_init_sta ();
      break;

    case NetworkType::AP_STA:
      this->network_init_apsta ();
      break;

    default:
#ifdef NETWORK_DEBUG
      debug_error ("Network type NONE");
#endif  
      break;
  }
}

void Network::network_stop (void)
{
  std::lock_guard<std::mutex> lock (network_mutex);

  switch ( network_type )
  {
    case NetworkType::AP:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI AP stop");
#endif  
      break;

    case NetworkType::STA:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI STA stop");
#endif  
      break;

    case NetworkType::AP_STA:
#ifdef NETWORK_DEBUG
      debug_normal ("WIFI STA+AP stop");
#endif  
      break;

    default:
      break;
  }

  if ( nullptr == ap_netif || nullptr == sta_netif || NetworkType::NONE == network_type )
  {
#ifdef NETWORK_DEBUG
    debug_error ("Failed: stop network interface");
#endif  
    return;
  }

  esp_wifi_disconnect ();
  esp_wifi_stop ();
}

const char* get_wifi_disconnection_string (wifi_err_reason_t wifi_err_reason)
{
  switch ( wifi_err_reason )
  {
    case WIFI_REASON_UNSPECIFIED:
      return "WIFI_REASON_UNSPECIFIED";
    case WIFI_REASON_AUTH_EXPIRE:
      return "WIFI_REASON_AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE:
      return "WIFI_REASON_AUTH_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE:
      return "WIFI_REASON_ASSOC_EXPIRE";
    case WIFI_REASON_ASSOC_TOOMANY:
      return "WIFI_REASON_ASSOC_TOOMANY";
    case WIFI_REASON_NOT_AUTHED:
      return "WIFI_REASON_NOT_AUTHED";
    case WIFI_REASON_NOT_ASSOCED:
      return "WIFI_REASON_NOT_ASSOCED";
    case WIFI_REASON_ASSOC_LEAVE:
      return "WIFI_REASON_ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_NOT_AUTHED:
      return "WIFI_REASON_ASSOC_NOT_AUTHED";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD:
      return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
      return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_BSS_TRANSITION_DISASSOC:
      return "WIFI_REASON_BSS_TRANSITION_DISASSOC";
    case WIFI_REASON_IE_INVALID:
      return "WIFI_REASON_IE_INVALID";
    case WIFI_REASON_MIC_FAILURE:
      return "WIFI_REASON_MIC_FAILURE";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
      return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
      return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
      return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID:
      return "WIFI_REASON_GROUP_CIPHER_INVALID";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
      return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_AKMP_INVALID:
      return "WIFI_REASON_AKMP_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
      return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP:
      return "WIFI_REASON_INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED:
      return "WIFI_REASON_802_1X_AUTH_FAILED";
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
      return "WIFI_REASON_CIPHER_SUITE_REJECTED";
    case WIFI_REASON_TDLS_PEER_UNREACHABLE:
      return "WIFI_REASON_TDLS_PEER_UNREACHABLE";
    case WIFI_REASON_TDLS_UNSPECIFIED:
      return "WIFI_REASON_TDLS_UNSPECIFIED";
    case WIFI_REASON_SSP_REQUESTED_DISASSOC:
      return "WIFI_REASON_SSP_REQUESTED_DISASSOC";
    case WIFI_REASON_NO_SSP_ROAMING_AGREEMENT:
      return "WIFI_REASON_NO_SSP_ROAMING_AGREEMENT";
    case WIFI_REASON_BAD_CIPHER_OR_AKM:
      return "WIFI_REASON_BAD_CIPHER_OR_AKM";
    case WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION:
      return "WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION";
    case WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS:
      return "WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS";
    case WIFI_REASON_UNSPECIFIED_QOS:
      return "WIFI_REASON_UNSPECIFIED_QOS";
    case WIFI_REASON_NOT_ENOUGH_BANDWIDTH:
      return "WIFI_REASON_NOT_ENOUGH_BANDWIDTH";
    case WIFI_REASON_MISSING_ACKS:
      return "WIFI_REASON_MISSING_ACKS";
    case WIFI_REASON_EXCEEDED_TXOP:
      return "WIFI_REASON_EXCEEDED_TXOP";
    case WIFI_REASON_STA_LEAVING:
      return "WIFI_REASON_STA_LEAVING";
    case WIFI_REASON_END_BA:
      return "WIFI_REASON_END_BA";
    case WIFI_REASON_UNKNOWN_BA:
      return "WIFI_REASON_UNKNOWN_BA";
    case WIFI_REASON_TIMEOUT:
      return "WIFI_REASON_TIMEOUT";
    case WIFI_REASON_PEER_INITIATED:
      return "WIFI_REASON_PEER_INITIATED";
    case WIFI_REASON_AP_INITIATED:
      return "WIFI_REASON_AP_INITIATED";
    case WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT:
      return "WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT";
    case WIFI_REASON_INVALID_PMKID:
      return "WIFI_REASON_INVALID_PMKID";
    case WIFI_REASON_INVALID_MDE:
      return "WIFI_REASON_INVALID_MDE";
    case WIFI_REASON_INVALID_FTE:
      return "WIFI_REASON_INVALID_FTE";
    case WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED:
      return "WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED";
    case WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED:
      return "WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED";
    case WIFI_REASON_BEACON_TIMEOUT:
      return "WIFI_REASON_BEACON_TIMEOUT";
    case WIFI_REASON_NO_AP_FOUND:
      return "WIFI_REASON_NO_AP_FOUND";
    case WIFI_REASON_AUTH_FAIL:
      return "WIFI_REASON_AUTH_FAIL";
    case WIFI_REASON_ASSOC_FAIL:
      return "WIFI_REASON_ASSOC_FAIL";
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
      return "WIFI_REASON_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_CONNECTION_FAIL:
      return "WIFI_REASON_CONNECTION_FAIL";
    case WIFI_REASON_AP_TSF_RESET:
      return "WIFI_REASON_AP_TSF_RESET";
    case WIFI_REASON_ROAMING:
      return "WIFI_REASON_ROAMING";
    case WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG:
      return "WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG";
    case WIFI_REASON_SA_QUERY_TIMEOUT:
      return "WIFI_REASON_SA_QUERY_TIMEOUT";
  }
  return "UNKNOWN";
}