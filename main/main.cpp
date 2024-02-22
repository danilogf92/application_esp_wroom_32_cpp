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


extern "C" {
  void app_main (void);
}

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
  debug_warning ("value sensor 1 is : %f", value);
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
  float value = ( rand () ) % 101;
  debug_warning ("value sensor 2 is : %f", value);
  return value;
}

Application app ("Esp");

void app_main (void)
{
  debug_normal ("Device name: %s", ( app.get_device_name () ).c_str ());

  // Outputs
  Output* motor_2 = new Output (GPIO_NUM_2, "Motor 2");
  app.add_output (motor_2);

  Output* motor_5 = new Output (GPIO_NUM_5, "Motor 5");
  app.add_output (motor_5);

  // Inputs
  Input* button_1 = new Input (GPIO_NUM_0, "Button 1");
  app.add_input (button_1);

  Input* button_2 = new Input (GPIO_NUM_1, "Button 2");
  app.add_input (button_2);

  // Sensors 
  Sensor* sensor_1 = new Sensor ("Sensor 1", sensor_1_config, update_value_sensor_1, 2000);
  app.add_sensor (sensor_1);
  Sensor* sensor_2 = new Sensor ("Sensor 2", sensor_2_config, update_value_sensor_2, 1000);
  app.add_sensor (sensor_2);

  // Device Info
  app.print_mac_address ();
  app.print_device_details ();
  app.print_device_details ();
  app.remove_output ("Motor 5");
  app.remove_output ("Motor 2");
  app.remove_input ("Button 2");
  print_new_line ();
  app.print_device_details ();


  // Network 
  Network* network = new Network ("esp32_network", "Hpalmira17");
  network->start (NetworkType::AP);
  // Network* network = new Network ("hPalmiraCM", "Hpalmira17");
  // network->start (NetworkType::STA);
  network->print_credentials ();
  // network->stop ();

  init_end_points ();

  // Add a minimum delay of 100 ms to prevent the watchdog......
  while ( true )
  {
    // app.set_output ("Motor 2", ON);
    // debug_normal ("Button 1 status %s", debug_get_bool_status (app.get_input_status ("Button 1")));
    // debug_normal ("Sensor 1 value            = %.2f", app.get_sensor_data ("Sensor 1"));
    // debug_normal ("Sensor 1 filtered value   = %.2f", app.get_sensor_data ("Sensor 1", true));
    // DELAY_SECONDS (COMMON_DATA_SECOND * 1);


    // debug_error ("Network clientes   = %d", network->get_clients_ap ());
    // app.set_output ("Motor 2", OFF);
    DELAY_SECONDS (COMMON_DATA_SECOND * 1);


    // Debug functions
    // debug_normal ("TEST NORMAL %d %0.2f %s %s", 5, 6.65, "Machine", "testing");
    // debug_warning ("TEST WARNING %d %0.2f %s %s", 5, 6.65, "Machine", "testing");
    // debug_error ("TEST ERROR %d %0.2f %s %s", 5, 6.65, "Machine", "testing");

    //DELAY_SECONDS (COMMON_DATA_MINIMUM);
  }
}



