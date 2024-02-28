#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <esp_err.h>

#include "Application.hpp"
#include "Output.hpp"
#include "IOutput.hpp"
#include "Input.hpp"
#include "Sensor.hpp"
#include "ultrasonic.h"
#include "debug.h"
#include "common_data.h"

extern "C" {
  void app_main (void);
}

#define MAX_DISTAMCE 500

static ultrasonic_sensor_t ultrasonic_1 = {
.trigger_pin = GPIO_NUM_5,
.echo_pin = GPIO_NUM_17
};

esp_err_t cb_config_sensor_1 ()
{
  esp_err_t error = ultrasonic_init (&ultrasonic_1);
  return error;
}

float get_data_sensor_1 ()
{
  float distance;
  esp_err_t resp = ultrasonic_measure (&ultrasonic_1, MAX_DISTAMCE, &distance);

  if ( ESP_OK != resp )
  {
    switch ( resp )
    {
      case ESP_ERR_ULTRASONIC_PING:
        debug_error ("ESP_ERR_ULTRASONIC_PING");
        break;

      case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
        debug_error ("ESP_ERR_ULTRASONIC_ECHO_TIMEOUT");
        break;

      case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
        debug_error ("ESP_ERR_ULTRASONIC_PING_TIMEOUT");
        break;

      default:
        break;
    }
  }
  return distance * 100;
}

void other_task (void* arg)
{
  Application* app_instance = reinterpret_cast< Application* >( arg );
  // std::function<float ()> _fn = sensor_instance->fn;

  while ( true )
  {
    app_instance->print_device_details ();
    DELAY (ONE_SECOND);
  }
}

void app_main (void)
{
  // Create application
  Application* app = Application::get_instance ("New_device");

  // Outputs
  IOutput* output_1 = OutputFactory::create_output (GPIO_NUM_2, "Led_2");
  app->add_output (output_1);

  IOutput* output_2 = OutputFactory::create_output (GPIO_NUM_4, "Led_3");
  app->add_output (output_2);

  // Inputs
  Input* input_1 = InputFactory::create_input (GPIO_NUM_0, "Button_0");
  app->add_input (input_1);
  Input* input_2 = InputFactory::create_input (GPIO_NUM_5, "Button_1");
  app->add_input (input_2);

  // Sensors
  Sensor* sensor_1 = SensorFactory::create_sensor ("Ultrasonic_1", cb_config_sensor_1, get_data_sensor_1, 50);
  app->add_sensor (sensor_1);

  // Network
  Network* network = NetworkFactory::create_network_ap_sta ("first_network", "danilo_tech", "hPalmiraCM", "Hpalmira17");
  app->add_network (network, NetworkType::AP_STA);
  // app->add_network (network, NetworkType::AP);
  // app->add_network (network, NetworkType::STA);

  app->print_device_details ();

  xTaskCreatePinnedToCore (other_task, "other_task", 2048 * 2, app, 2, nullptr, 1);


  // Add a minimum delay of 100 ms to prevent the watchdog......
  while ( true )
  {
    app->set_output ("Led_2", ON);
    debug_warning ("Button_0 status %d", app->get_input_status ("Button_0")); // Button_0 status
    debug_normal ("AP clients %d", network->get_clients_ap ()); // Clients

    debug_normal ("Ultrasonic_1 normal        = %0.2f", app->get_sensor_data ("Ultrasonic_1", SensorFilterType::NONE)); // normal
    debug_normal ("Ultrasonic_1 median filter = %0.2f", app->get_sensor_data ("Ultrasonic_1", SensorFilterType::MEDIAN)); // median filter
    debug_normal ("Ultrasonic_1 Kalman filter = %0.2f", app->get_sensor_data ("Ultrasonic_1", SensorFilterType::KALMAN)); // kalman filter

    DELAY (ONE_SECOND);
    // DELAY (MINIMUM);
  }
}



