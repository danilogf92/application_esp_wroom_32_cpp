#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>

#include "Application.hpp"
#include "Input.hpp"
#include "Output.hpp"
#include "Sensor.hpp"
#include "Network.hpp"
#include "Server.h"
#include "debug.h"
#include "common_data.h"
#include <ultrasonic.h>

// static const char* TAG = "Application";

extern "C" {
  void app_main (void);
}

#define MAX_DISTANCE_CM 500 // 5m max

gpio_num_t TRIGGER_GPIO = GPIO_NUM_5;
gpio_num_t ECHO_GPIO = GPIO_NUM_17;

esp_err_t sensor_1_config ()
{
  // config
  // -------------------  
  //--------------------

  debug_warning ("Config your sensor 1 here");
  return ESP_OK;
}

float update_value_sensor_1 (void)
{
  float value = ( rand () ) % 101;
  // debug_warning ("value sensor 1 is : %f", value);
  return value;
}

esp_err_t sensor_2_config ()
{
  // config
  // -------------------  
  //--------------------

  debug_warning ("Config your sensor 2 here");
  return ESP_OK;
}

float update_value_sensor_2 (void)
{
  float value = ( rand () ) % 51;
  // debug_warning ("value sensor 2 is : %f", value);
  return value;
}

static ultrasonic_sensor_t ultrasonic_1 = {
  .trigger_pin = TRIGGER_GPIO,
  .echo_pin = ECHO_GPIO
};

esp_err_t sensor_3_config ()
{
  ultrasonic_init (&ultrasonic_1);
  debug_warning ("Config your sensor 3 here");
  return ESP_OK;
}

float update_value_sensor_3 (void)
{
  float distance;
  esp_err_t res = ultrasonic_measure (&ultrasonic_1, MAX_DISTANCE_CM, &distance);
  if ( res != ESP_OK )
  {
    debug_error ("Error %d: ", res);
    switch ( res )
    {
      case ESP_ERR_ULTRASONIC_PING:
        debug_normal ("Cannot ping (device is in invalid state)");
        break;
      case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
        debug_normal ("Ping timeout (no device found)");
        break;
      case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
        debug_normal ("Echo timeout (i.e. distance too big)");
        break;
      default:
        debug_warning ("%s", esp_err_to_name (res));
    }
  }
  else
  {
    //    debug_normal ("Distance: %0.04f cm", distance * 100);
  }
  return ( float ) ( distance * 100.0f );
}

Application* app = Application::get_instance ("Esp");

void app_main (void)
{
  debug_normal ("Device name: %s", ( app->get_device_name () ).c_str ());

  // Outputs
  Output* motor_2 = OutputFactory::create_output (GPIO_NUM_2, "Motor 2");
  app->add_output (motor_2);

  Output* motor_5 = OutputFactory::create_output (GPIO_NUM_5, "Motor 5");
  app->add_output (motor_5);

  // Inputs
  Input* button_1 = InputFactory::create_input (GPIO_NUM_0, "Button 1");
  app->add_input (button_1);

  Input* button_2 = InputFactory::create_input (GPIO_NUM_1, "Button 2");
  app->add_input (button_2);

  // Sensors 
  Sensor* sensor_1 = SensorFactory::create_sensor ("Sensor 1", sensor_1_config, update_value_sensor_1, 2000);
  app->add_sensor (sensor_1);
  Sensor* sensor_2 = SensorFactory::create_sensor ("Sensor 2", sensor_2_config, update_value_sensor_2, 1000);
  app->add_sensor (sensor_2);

  Sensor* sensor_3 = SensorFactory::create_sensor ("Sensor 3", sensor_3_config, update_value_sensor_3, 500);
  app->add_sensor (sensor_3);

  // Device Info
  app->print_mac_address ();
  app->print_device_details ();
  app->print_device_details ();
  app->remove_output ("Motor 5");
  // app->remove_output ("Motor 2");
  app->remove_input ("Button 2");
  // app->remove_sensor ("Sensor 3");
  print_new_line ();


  // Network 
  Network* network_base = NetworkFactory::create_network ("danilo_tech_ap", "danilo_tech");
  app->add_network (network_base, NetworkType::AP);

  // Network* network_base = NetworkFactory::create_network ("hPalmiraCM", "Hpalmira17");
  // app->add_network (network_base, NetworkType::STA);
  // app->stop_network ();

  if ( ESP_OK == app->network_exist () )
  {
    init_end_points ();
  }

  app->print_device_details ();

  // Add a minimum delay of 100 ms to prevent the watchdog......
  while ( true )
  {
    app->set_output ("Motor 2", ON);
    debug_normal ("Button 1 status %s", debug_get_bool_status (app->get_input_status ("Button 1")));
    debug_normal ("Sensor 1 value            = %.2f", app->get_sensor_data ("Sensor 1"));
    debug_normal ("Sensor 2 value            = %.2f", app->get_sensor_data ("Sensor 2"));
    debug_normal ("Sensor 3 value            = %.2f", app->get_sensor_data ("Sensor 3"));
    debug_warning ("Sensor 3 mean filter       = %.2f", app->get_sensor_data ("Sensor 3", SensorFilterType::MEDIAN));
    debug_error ("Sensor 3 kalman filter    = %.2f", app->get_sensor_data ("Sensor 3", SensorFilterType::KALMAN));

    server_data.distance = app->get_sensor_data ("Sensor 3", SensorFilterType::KALMAN);
    DELAY_SECONDS (COMMON_DATA_SECOND * 1);


    debug_error ("Network clients   = %d", network_base->get_clients_ap ());
    app->set_output ("Motor 2", OFF);
    DELAY_SECONDS (COMMON_DATA_SECOND * 1);


    // Debug functions
    // debug_normal ("TEST NORMAL %d %0.2f %s %s", 5, 6.65, "Machine", "testing");
    // debug_warning ("TEST WARNING %d %0.2f %s %s", 5, 6.65, "Machine", "testing");
    // debug_error ("TEST ERROR %d %0.2f %s %s", 5, 6.65, "Machine", "testing");

    DELAY_SECONDS (COMMON_DATA_MINIMUM);
  }
}



