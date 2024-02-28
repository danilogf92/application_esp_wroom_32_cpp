#include "Network.hpp"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

const char* TAG = "Network";
constexpr uint8_t MAX_ATTEMMPS = 5;

static bool attempt_reconnect = false;
static EventGroupHandle_t wifi_events;
static int CONNECTED = BIT0;
static int DISCONNECTED = BIT1;

static const char* get_wifi_disconnection_string (wifi_err_reason_t wifi_err_reason);

// Network::Network (std::string _ssid, std::string _password, uint16_t timeout) :
//   ap_ssid (_ssid), ap_password (_password), sta_ssid (_ssid), sta_password (_password), timeout (timeout)
// {
//   clients_connected = 0;
//   disconnection_err_count = 0;
//   // esp_err_t err = nvs_flash_init ();
//   // if ( err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND )
//   // {
//   //   nvs_flash_erase ();
//   //   nvs_flash_init ();
//   // }

//   // ESP_ERROR_CHECK (esp_netif_init ());
//   // ESP_ERROR_CHECK (esp_event_loop_create_default ());
//   wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT ();
//   ESP_ERROR_CHECK (esp_wifi_init (&wifi_init_config));
//   ESP_ERROR_CHECK (esp_event_handler_register (WIFI_EVENT, ESP_EVENT_ANY_ID, handle_wifi_event, this));
//   ESP_ERROR_CHECK (esp_event_handler_register (IP_EVENT, IP_EVENT_STA_GOT_IP, handle_wifi_event, this));

//   ESP_LOGE (TAG, "Primer constructor");
// }

Network::Network (std::string _ap_ssid, std::string _ap_password, std::string _sta_ssid, std::string _sta_password, uint16_t timeout) :
  ap_ssid (_ap_ssid), ap_password (_ap_password), sta_ssid (_sta_ssid), sta_password (_sta_password), timeout (timeout), loop_active (false)
{
  clients_connected = 0;
  disconnection_err_count = 0;
  ap_netif = NULL;
  sta_netif = NULL;
  esp_err_t err = nvs_flash_init ();
  if ( err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND )
  {
    nvs_flash_erase ();
    nvs_flash_init ();
  }

  // ESP_ERROR_CHECK (esp_netif_init ());
  // ESP_ERROR_CHECK (esp_event_loop_create_default ());
  // wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT ();
  // ESP_ERROR_CHECK (esp_wifi_init (&wifi_init_config));
  ESP_LOGE (TAG, "constructor");
}

Network::~Network ()
{
  //this->stop ();
  ESP_LOGE (TAG, "Delete constructor");
}

void Network::print_credentials (void)
{
  ESP_LOGI (TAG, "Network: ssid=%s, password=%s", ap_ssid.c_str (), ap_password.c_str ());
}

int8_t Network::get_clients_ap (void)
{
  if ( NetworkType::AP == network_type || NetworkType::AP_STA == network_type )
  {
    return clients_connected;
  }
  return -1;
}

void Network::handle_wifi_event (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  ESP_LOGE (TAG, "handle_wifi_event");

  Network* obj = reinterpret_cast< Network* >( arg );

  ESP_LOGI (TAG, "Attemmps = %d", obj->disconnection_err_count);

  switch ( event_id )
  {
    case WIFI_EVENT_STA_START:
      ESP_LOGI (TAG, "WIFI_EVENT_STA_START");
      esp_wifi_connect ();
      break;

    case WIFI_EVENT_STA_CONNECTED:
      ESP_LOGI (TAG, "WIFI_EVENT_STA_CONNECTED");
      obj->disconnection_err_count = 0;
      break;

    case WIFI_EVENT_STA_DISCONNECTED:
      {
        wifi_event_sta_disconnected_t* wifi_event_sta_disconnected = ( wifi_event_sta_disconnected_t* ) event_data;
        ESP_LOGW (TAG, "DISCONNECTED %d: %s", wifi_event_sta_disconnected->reason, get_wifi_disconnection_string (( wifi_err_reason_t ) wifi_event_sta_disconnected->reason));

        if ( attempt_reconnect )
        {
          if ( wifi_event_sta_disconnected->reason == WIFI_REASON_NO_AP_FOUND ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_ASSOC_LEAVE ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_AUTH_EXPIRE ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_AUTH_FAIL ||
            wifi_event_sta_disconnected->reason == WIFI_REASON_CONNECTION_FAIL )
          {

            if ( obj->disconnection_err_count++ < MAX_ATTEMMPS )
            {
              vTaskDelay (pdMS_TO_TICKS (5000));
              esp_wifi_connect ();
              break;
            }
            else
            {
              ESP_LOGE (TAG, "WIFI retries exceeded");
              esp_restart ();
            }
          }
        }

        xEventGroupSetBits (wifi_events, DISCONNECTED);
        break;
      }

    case IP_EVENT_STA_GOT_IP:
      ESP_LOGI (TAG, "IP_EVENT_STA_GOT_IP");
      xEventGroupSetBits (wifi_events, CONNECTED);
      break;

    case WIFI_EVENT_SCAN_DONE:
      ESP_LOGW (TAG, "WIFI_EVENT_SCAN_DONE");

      break;
    case WIFI_EVENT_STA_STOP:
      ESP_LOGI (TAG, "WIFI_EVENT_STA_STOP");
      break;

    case WIFI_EVENT_AP_START:
      ESP_LOGI (TAG, "WIFI_EVENT_AP_START");
      break;
    case WIFI_EVENT_AP_STOP:
      ESP_LOGI (TAG, "WIFI_EVENT_AP_STOP");
      break;
    case WIFI_EVENT_AP_STACONNECTED:
      obj->clients_connected++;
      ESP_LOGI (TAG, "WIFI_EVENT_AP_STACONNECTED, Clients = %d", obj->clients_connected);
      break;
    case  WIFI_EVENT_AP_STADISCONNECTED:
      obj->clients_connected--;
      ESP_LOGI (TAG, "WIFI_EVENT_AP_STADISCONNECTED, Clients = %d", obj->clients_connected);
      break;
    case WIFI_EVENT_AP_PROBEREQRECVED:
      ESP_LOGI (TAG, "WIFI_EVENT_AP_PROBEREQRECVED");
      break;

    default:
      break;
  }
}

void Network::wifi_init_ap (void)
{
  ESP_ERROR_CHECK (esp_netif_init ());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT ();
  ESP_ERROR_CHECK (esp_wifi_init (&cfg));

  if ( !loop_active )
  {
    ESP_ERROR_CHECK (esp_event_loop_create_default ());
    loop_active = true;
  }

  ESP_LOGE (TAG, "wifi_init_ap");

  ap_netif = esp_netif_create_default_wifi_ap ();
  if ( NULL == ap_netif )
  {
    ESP_LOGE (TAG, "Failed to create STA interface");
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

void Network::wifi_stop_ap ()
{
  esp_wifi_stop ();
  esp_wifi_deinit ();
  esp_event_loop_delete_default ();

  if ( NULL != ap_netif )
  {
    esp_netif_destroy (ap_netif);
    ap_netif = NULL;
  }
  loop_active = false;
}

esp_err_t Network::wifi_init_sta (void)
{
  ESP_ERROR_CHECK (esp_netif_init ());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT ();
  ESP_ERROR_CHECK (esp_wifi_init (&cfg));

  if ( !loop_active )
  {
    ESP_ERROR_CHECK (esp_event_loop_create_default ());
    loop_active = true;
  }

  ESP_LOGE (TAG, "wifi_connect_sta");
  attempt_reconnect = true;
  wifi_events = xEventGroupCreate ();

  sta_netif = esp_netif_create_default_wifi_sta ();
  if ( NULL == sta_netif )
  {
    ESP_LOGE (TAG, "Failed to create STA interface");
    return ESP_FAIL;
  }

  ESP_ERROR_CHECK (esp_event_handler_register (WIFI_EVENT, ESP_EVENT_ANY_ID, handle_wifi_event, this));
  ESP_ERROR_CHECK (esp_event_handler_register (IP_EVENT, IP_EVENT_STA_GOT_IP, handle_wifi_event, this));

  ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_STA));
  wifi_config_t wifi_config = {};
  strcpy (reinterpret_cast< char* >( wifi_config.sta.ssid ), sta_ssid.c_str ());
  strcpy (reinterpret_cast< char* >( wifi_config.sta.password ), sta_password.c_str ());
  ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK (esp_wifi_start ());
  EventBits_t result = xEventGroupWaitBits (wifi_events, CONNECTED | DISCONNECTED, true, false, pdMS_TO_TICKS (timeout));
  if ( result == CONNECTED )
    return ESP_OK;
  return ESP_FAIL;
}

void Network::wifi_stop_sta ()
{
  esp_wifi_stop ();
  esp_event_handler_unregister (WIFI_EVENT, ESP_EVENT_ANY_ID, handle_wifi_event);
  esp_event_handler_unregister (IP_EVENT, IP_EVENT_STA_GOT_IP, handle_wifi_event);
  esp_wifi_deinit ();
  esp_event_loop_delete_default ();

  if ( NULL != sta_netif )
  {
    esp_netif_destroy (sta_netif);
    sta_netif = NULL;
  }

  loop_active = false;
}

void Network::network_connection_type (NetworkType type)
{
  ESP_LOGE (TAG, "WIFI start");

  network_type = type;
  switch ( type )
  {
    case NetworkType::AP:
      this->wifi_init_ap ();
      break;

    case NetworkType::STA:
      this->wifi_init_sta ();
      break;

    case NetworkType::AP_STA:
      this->wifi_init_sta ();
      this->wifi_init_ap ();
      break;

    default:
      break;
  }
}

// void Network::start_ap_sta ()
// {
//   ESP_LOGE (TAG, "start_ap_sta");

//   if ( nullptr == ap_netif )
//   {
//     ap_netif = esp_netif_create_default_wifi_ap ();
//   }

//   attempt_reconnect = true;
//   wifi_events = xEventGroupCreate ();

//   if ( nullptr == sta_netif )
//   {
//     sta_netif = esp_netif_create_default_wifi_sta ();
//   }

//   ESP_ERROR_CHECK (esp_wifi_set_mode (WIFI_MODE_APSTA));

//   wifi_config_t ap_config = {
//         .ap = {
//             .ssid = "",
//             .password = "",
//             .ssid_len = 0,
//             .channel = 4,
//             .authmode = WIFI_AUTH_WPA_WPA2_PSK,
//             .max_connection = 4,
//             .beacon_interval = 100,
//         }
//   };
//   strcpy (reinterpret_cast< char* >( ap_config.ap.ssid ), ap_ssid.c_str ());
//   strcpy (reinterpret_cast< char* >( ap_config.ap.password ), ap_password.c_str ());

//   ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_AP, &ap_config));

//   wifi_config_t sta_config = {};

//   strcpy (reinterpret_cast< char* >( sta_config.sta.ssid ), sta_ssid.c_str ());
//   strcpy (reinterpret_cast< char* >( sta_config.sta.password ), sta_password.c_str ());

//   ESP_ERROR_CHECK (esp_wifi_set_config (WIFI_IF_STA, &sta_config));
//   ESP_ERROR_CHECK (esp_wifi_start ());
// }

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