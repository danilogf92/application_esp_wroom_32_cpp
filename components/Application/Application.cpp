#include "Application.hpp"
#include <mutex>

// #define APPLICATION_DEBUG

Application* Application::application_instance = nullptr;
std::mutex application_mutex;

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
  debug_normal ("Device name %s", device_name.c_str ());
  debug_normal ("CONSTRUCTOR, Application ");
#endif
}

Application* Application::get_instance (const std::string& value)
{
  if ( nullptr == application_instance )
  {
    application_instance = new Application (value);
  }
  return application_instance;
}

Application::~Application ()
{
  if ( nullptr != application_instance )
  {
    outputs.clear ();
    inputs.clear ();
    sensors.clear ();
  }
#ifdef APPLICATION_DEBUG
  print_new_line ();
  debug_error ("Device name %s", device_name.c_str ());
  debug_error ("DESTRUCTOR, Application ");
#endif
}

int8_t Application::get_clients_connected (void)
{
#ifdef APPLICATION_DEBUG
  debug_normal ("Clients connected %d", network->network_get_clients_connected ());
#endif

  if ( nullptr != network )
  {
    return network->network_get_clients_connected ();
  }
#ifdef APPLICATION_DEBUG
  debug_normal ("AP or AP_STA mode is required");
#endif
  return -1;
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
  debug_normal ("Clients connected \"%d\"", network->network_get_clients_connected ());

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

void Application::add_output (Output* output)
{
  outputs.push_back (std::unique_ptr<Output> (output));
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

void Application::add_input (Input* input)
{
  inputs.push_back (std::unique_ptr <Input> (input));

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

void Application::add_network (const Network* _network)
{
  if ( !_network )
  {
    debug_error ("Network pointer error");
    return;
  }
  network = std::unique_ptr<Network> (const_cast< Network* >( _network ));
}

void Application::start_network (NetworkType _type)
{
  std::lock_guard<std::mutex> lock (application_mutex);

  if ( !network )
  {
    debug_error ("Start Network error");
    return;
  }

  network->network_connection_manager (_type);
  network_active = true;
}

void Application::stop_network (void)
{
  std::lock_guard<std::mutex> lock (application_mutex);

  if ( !network )
  {
    debug_error ("Stop Network error");
    return;
  }
  network->network_stop ();
  network_active = false;
}

esp_err_t Application::network_exist (void)
{
  if ( network_active )
  {
    return ESP_OK;
  }
  return ESP_FAIL;
}
