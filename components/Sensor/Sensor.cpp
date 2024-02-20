#include <cstdlib>
#include <iostream>
#include <cstring>

#include "Sensor.hpp"
#include "debug.h"

// #define SENSOR_DEBUG

constexpr int STACK_DEPTH = 2408;
constexpr int PRIORITY = 5;

Sensor::Sensor (std::string _name, callback_config_function callback_config, std::function<float ()> func, uint16_t samples) : value (0), sampling_period (samples), enable_sensor (true), name (_name), filtered_value (0), filter (10, 0), fn (func)
{
  ( *callback_config )( );

#ifdef SENSOR_DEBUG
  debug_warning ("Sensor %s Initialized", name.c_str ());
#endif
}

Sensor::~Sensor ()
{
  enable_sensor = false;
  if ( nullptr != task_handle )
  {
    vTaskDelete (task_handle);
    task_handle = nullptr;
}
  }

float Sensor::get_value ()
{
  return value;
}

void Sensor::update_value (float new_value)
{
  value = new_value;
  filter.push_back (new_value);
  filter.erase (filter.begin ());

#ifdef SENSOR_DEBUG
  debug_warning ("update_value() value is : %0.2f", new_value);
#endif
}

float Sensor::get_filtered_value ()
{
  // TODO Test filter
  float _filter = 0;

  for ( float filter_item : filter )
  {
    _filter += filter_item;

#ifdef SENSOR_DEBUG
    debug_normal ("vector : %0.2f, size: %d", filter_item, filter.size ());
#endif
  }

  _filter = _filter / filter.size ();

  filtered_value = _filter;

#ifdef SENSOR_DEBUG
  debug_warning ("filter_value() value is : %0.2f", filtered_value);
#endif

  return filtered_value;
}

std::string Sensor::get_name () const
{
  return name;
}

esp_err_t Sensor::enable (void)
{
  enable_sensor = true;

  if ( nullptr == task_handle )
  {
    this->init ();
  }

#ifdef SENSOR_DEBUG
  debug_warning ("Sensor \"%s\" enabled", name.c_str ());
#endif
  return ESP_OK;
}
esp_err_t Sensor::disable (void)
{
  enable_sensor = false;
  if ( nullptr != task_handle )
  {
    vTaskDelete (task_handle);
    task_handle = nullptr;
  }

#ifdef SENSOR_DEBUG
  debug_warning ("Sensor \"%s\" disabled", name.c_str ());
#endif
  return ESP_OK;
}

uint16_t Sensor::get_sample_period ()
{
  return  sampling_period;
}

void Sensor::sensor_task (void* arg)
{
  Sensor* sensor_instance = reinterpret_cast< Sensor* >( arg );
  std::function<float ()> _fn = sensor_instance->fn;

  while ( true )
  {

#ifdef SENSOR_DEBUG
    debug_warning ("Sensor \"%s\" Sampling period is: %d %f", ( sensor_instance->name ).c_str (), sensor_instance->sampling_period, _fn ());
#endif

    sensor_instance->update_value (_fn ());

    vTaskDelay (pdMS_TO_TICKS (sensor_instance->sampling_period));
  }
}

void Sensor::init (void)
{
  xTaskCreatePinnedToCore (sensor_task, "sensor_task", STACK_DEPTH, this, PRIORITY, &task_handle, 0);
}