#include "Server.h"

static const char* TAG = "SERVER";
static httpd_handle_t server = NULL;

server_data_t server_data = {
  .led = GPIO_NUM_2,
  .temperature = 0,
  .distance = 0
};

static void toggle_output (gpio_num_t _pin, bool is_on)
{
  gpio_set_level (_pin, is_on);
}

static esp_err_t initialize_output (gpio_num_t _pin)
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = ( 1ULL << _pin );
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  esp_err_t err = gpio_config (&io_conf);
  return err;
}

static esp_err_t on_get_temperature (httpd_req_t* req)
{
  ESP_LOGI (TAG, "url %s was hit", req->uri);
  srand (time (NULL));
  server_data.temperature = ( ( rand () % 4101 ) / 100.0 ) + 10;

  cJSON* root = cJSON_CreateObject ();
  if ( root == NULL )
  {
    httpd_resp_send_err (req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON object");
    return ESP_FAIL;
  }

  cJSON_AddNumberToObject (root, "temperature", server_data.temperature);

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

static esp_err_t on_get_distance (httpd_req_t* req)
{
  ESP_LOGI (TAG, "url %s was hit", req->uri);
  srand (time (NULL));

  cJSON* root = cJSON_CreateObject ();
  if ( root == NULL )
  {
    httpd_resp_send_err (req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON object");
    return ESP_FAIL;
  }

  cJSON_AddNumberToObject (root, "distance", server_data.distance);

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
  toggle_output (server_data.led, is_on);
  httpd_resp_set_status (req, "204 NO CONTENT");
  httpd_resp_send (req, NULL, 0);
  return ESP_OK;
}

static esp_err_t on_hello_world_url (httpd_req_t* req)
{
  cJSON* root = cJSON_CreateObject ();
  if ( root == NULL )
  {
    httpd_resp_send_err (req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON object");
    return ESP_FAIL;
  }

  cJSON_AddStringToObject (root, "msg", "hello world from esp32😀!");

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

void init_end_points (void)
{
  ESP_LOGI (TAG, "Server start");

  ESP_ERROR_CHECK (initialize_output (server_data.led));
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

  httpd_uri_t get_distance = {
  .uri = "/api/distance",
  .method = HTTP_GET,
  .handler = on_get_distance };
  httpd_register_uri_handler (server, &get_distance);
}
