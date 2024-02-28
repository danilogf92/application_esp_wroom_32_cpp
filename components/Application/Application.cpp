#include <iostream>
#include <cstring> 

#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_err.h"

#include "Application.hpp"
#include "debug.h"
#include "esp_mac.h"

// #define APPLICATION_DEBUG

Application* Application::application_ = nullptr;

static std::string get_mac (void)
{
  uint8_t mac[6];
  char _mac_address[18];
  esp_err_t ret = esp_efuse_mac_get_default (mac);
  if ( ESP_FAIL == ret )
  {

#ifdef APPLICATION_DEBUG
    debug_error ("Not found mac address");
#endif

    return std::string ("NOT FOUND");
  }
  else
  {
    snprintf (_mac_address, sizeof (_mac_address), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

#ifdef APPLICATION_DEBUG
    debug_warning ("Mac address %s", _mac_address);
#endif

    return std::string (_mac_address);
  }
}

Application::Application (std::string _name)
  : device_name (_name), mac_address (), sensors (), network_active (false)
{
  mac_address = get_mac ();
  device_name = device_name + "_" + mac_address;

#ifdef APPLICATION_DEBUG
  debug_warning ("Device name %s", device_name.c_str ());
  debug_warning ("Constructor, Application ");
#endif
}

Application* Application::get_instance (const std::string& value)
{
  if ( nullptr == application_ )
  {
    application_ = new Application (value);
  }
  return application_;
}


Application::~Application ()
{
  if ( nullptr != application_ )
  {
    delete application_;
    application_ = nullptr;
  }
  network.reset ();
}

void Application::print_mac_address (void)
{
  debug_normal ("Mac %s", mac_address.c_str ());
}

std::string Application::get_device_name (void) const
{
  return device_name;
}

void Application::print_device_details (void)
{
  print_new_line ();
  debug_normal ("Device name \"%s\"", device_name.c_str ());
  debug_normal ("Input number \"%d\"", inputs.size ());
  debug_normal ("Sensor number \"%d\"", sensors.size ());
  debug_normal ("Output number \"%d\"", outputs.size ());

  if ( network_active )
  {
    debug_normal ("Network Active");
  }
  else
  {
    debug_normal ("Network not initialized");
  }
  print_new_line ();
}

void Application::add_output (IOutput* output)
{
  outputs.push_back (std::unique_ptr<IOutput> (output));
}

void Application::set_output (std::string _name, bool _state)
{
  for ( auto iterator = outputs.begin (); iterator != outputs.end (); ++iterator )
  {
    if ( ( *iterator )->get_name () == _name )
    {
      ( *iterator )->set_pin (_state);
#ifdef APPLICATION_DEBUG
      debug_warning ("Output name \"%s\" set: \"%s\" ", _name.c_str (), debug_get_bool_status (_state));
#endif      
      return;
    }
  }
#ifdef APPLICATION_DEBUG
  debug_error ("Output with name \"%s\" not found!", _name.c_str ());
#endif  
}

void Application::remove_output (std::string _name)
{
  for ( auto iterator = outputs.begin (); iterator != outputs.end (); ++iterator )
  {
    if ( ( *iterator )->get_name () == _name )
    {
      iterator = outputs.erase (iterator);
#ifdef APPLICATION_DEBUG
      debug_warning ("Output name \"%s\" erased", _name.c_str ());
#endif
      return;
    }
  }
#ifdef APPLICATION_DEBUG
  debug_error ("Output with name \"%s\" not found!", _name.c_str ());
#endif
}

void Application::add_input (IInput* input)
{
  inputs.push_back (std::unique_ptr <IInput> (input));

#ifdef APPLICATION_DEBUG
  debug_warning ("Input name \"%s\" added", ( input->get_name () ).c_str ());
#endif  
}

void Application::remove_input (std::string _name)
{
  for ( auto iterator = inputs.begin (); iterator != inputs.end (); ++iterator )
  {
    if ( ( *iterator )->get_name () == _name )
    {
      inputs.erase (iterator);
#ifdef APPLICATION_DEBUG
      debug_warning ("Input name \"%s\" erased", _name.c_str ());
#endif   
      return;
    }
  }
#ifdef APPLICATION_DEBUG
  debug_error ("Input name \"%s\" not found!", _name.c_str ());
#endif  
}

bool Application::get_input_status (std::string _name)
{
  bool _state = false;
  for ( auto iterator = inputs.begin (); iterator != inputs.end (); ++iterator )
  {
    if ( ( *iterator )->get_name () == _name )
    {
      _state = ( *iterator )->get_status_pin ();
      return _state;
    }
  }
  return _state;
}

void Application::add_sensor (ISensor* sensor)
{
  sensors.push_back (std::unique_ptr<ISensor> (sensor));
  sensor->init ();

#ifdef APPLICATION_DEBUG
  debug_warning ("Sensor name \"%s\" added", ( sensor->get_name () ).c_str ());
#endif  
}

void Application::remove_sensor (std::string _name)
{
  for ( auto iterator = sensors.begin (); iterator != sensors.end (); ++iterator )
  {
    if ( ( *iterator )->get_name () == _name )
    {
      sensors.erase (iterator);
#ifdef APPLICATION_DEBUG
      debug_warning ("Sensor name \"%s\" erased", _name.c_str ());
#endif      
      return;
    }
  }
#ifdef APPLICATION_DEBUG
  debug_error ("Sensor with name \"%s\" not found!", _name.c_str ());
#endif  
}

float Application::get_sensor_data (std::string _name, SensorFilterType filter_type)
{
  float _value = -1;
  for ( auto iterator = sensors.begin (); iterator != sensors.end (); ++iterator )
  {
    if ( ( *iterator )->get_name () == _name )
    {
      switch ( filter_type )
      {
        case SensorFilterType::NONE:
          _value = ( ( *iterator )->get_value () );
          break;

        case SensorFilterType::MEDIAN:
          _value = ( ( *iterator )->get_median_value () );
          break;

        case SensorFilterType::KALMAN:
          _value = ( ( *iterator )->get_kalman_value () );
          break;

        default:
          break;
      }

#ifdef APPLICATION_DEBUG
      debug_warning ("Sensor name \"%s\"= %f", _name.c_str (), _value);
#endif      
      return _value;
    }
  }
#ifdef APPLICATION_DEBUG
  debug_error ("Sensor with name \"%s\" not found!", _name.c_str ());
#endif  
  return _value;
}

void Application::add_network (const Network* _network, NetworkType _type)
{
  if ( !_network )
  {
    debug_error ("Network pointer errror");
    return;
  }

  // if ( network )
  // {
  //   debug_warning ("Network delete");
  //   delete_network ();
  // }

  network = std::unique_ptr<Network> (const_cast< Network* >( _network ));
  // network = const_cast< Network* >( _network );
  network->network_connection_type (_type);
  network_active = true;
}


esp_err_t Application::delete_network (void)
{
  if ( network )
  {
    //network->stop ();
    // network.reset ();
    network_active = false;
    return ESP_OK;
  }
  return ESP_FAIL;
}

esp_err_t Application::network_exist (void)
{
  if ( network_active )
  {
    return ESP_OK;
  }
  return ESP_FAIL;
}
