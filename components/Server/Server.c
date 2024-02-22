#include "Server.h"

static const char* TAG = "SERVER";
static httpd_handle_t server = NULL;
#define LED 2


static void toggle_led (bool is_on)
{
  gpio_set_level (LED, is_on);
}

void InitializeLed ()
{
  gpio_set_direction (LED, GPIO_MODE_OUTPUT);
}

static esp_err_t on_get_temperature (httpd_req_t* req)
{
  ESP_LOGI (TAG, "url %s was hit", req->uri);

  cJSON* root = cJSON_CreateObject ();
  if ( root == NULL )
  {
    httpd_resp_send_err (req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON object");
    return ESP_FAIL;
  }

  cJSON_AddNumberToObject (root, "temperature", 25.5f);

  char* json_string = cJSON_PrintUnformatted (root);
  if ( json_string == NULL )
  {
    cJSON_Delete (root);
    httpd_resp_send_err (req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to generate JSON");
    return ESP_FAIL;
  }

  httpd_resp_send (req, json_string, strlen (json_string));

  cJSON_Delete (root);
  free (json_string);

  return ESP_OK;
}

static esp_err_t on_toggle_led_url (httpd_req_t* req)
{
  char buffer[100];
  memset (&buffer, 0, sizeof (buffer));
  httpd_req_recv (req, buffer, req->content_len);
  cJSON* payload = cJSON_Parse (buffer);
  cJSON* is_on_json = cJSON_GetObjectItem (payload, "isLedOn");
  bool is_on = cJSON_IsTrue (is_on_json);
  cJSON_Delete (payload);
  toggle_led (is_on);
  httpd_resp_set_status (req, "204 NO CONTENT");
  httpd_resp_send (req, NULL, 0);
  return ESP_OK;
}

static esp_err_t on_hello_world_url (httpd_req_t* req)
{
  httpd_resp_sendstr (req, "hello world!");
  return ESP_OK;
}

void init_end_points (void)
{
  ESP_LOGI (TAG, "Server start");

  InitializeLed ();
  httpd_config_t config = HTTPD_DEFAULT_CONFIG ();

  ESP_ERROR_CHECK (httpd_start (&server, &config));

  httpd_uri_t toggle_led_url = {
      .uri = "/api/led",
      .method = HTTP_POST,
      .handler = on_toggle_led_url };
  httpd_register_uri_handler (server, &toggle_led_url);

  httpd_uri_t hello_world_url = {
    .uri = "/api/hello-world",
    .method = HTTP_GET,
    .handler = on_hello_world_url };
  httpd_register_uri_handler (server, &hello_world_url);

  httpd_uri_t get_temperature = {
  .uri = "/api/temperature",
  .method = HTTP_GET,
  .handler = on_get_temperature };
  httpd_register_uri_handler (server, &get_temperature);
}










