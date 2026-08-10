#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MAX6675.h>

#include "TemperatureFilter.h"

class TemperatureManager
{
public:

    TemperatureManager(
        uint8_t oneWirePin,
        uint8_t thermoCLK,
        uint8_t thermoCS,
        uint8_t thermoDO
    );

    void begin();
    void update();

    // Individual filtered DS18B20 temperatures
    float getSensor1Temperature() const;
    float getSensor2Temperature() const;
    float getSensor3Temperature() const;

    // Filtered chamber average
    float getAverageTemperature() const;

    // Filtered MAX6675 temperature
    float getHotTemperature() const;

    // Number of detected DS18B20 sensors
    uint8_t getSensorCount() const;

private:

    // =====================================================
    // DS18B20
    // =====================================================

    OneWire oneWire;
    DallasTemperature ds18b20;

    uint8_t sensorCount;

    unsigned long lastDS18B20Request;
    bool ds18b20ConversionStarted;

    float sensor1Temperature;
    float sensor2Temperature;
    float sensor3Temperature;

    float averageTemperature;

    // =====================================================
    // MAX6675
    // =====================================================

    MAX6675 thermocouple;

    float hotTemperature;

    // =====================================================
    // FILTERS
    // =====================================================

    TemperatureFilter sensor1Filter;
    TemperatureFilter sensor2Filter;
    TemperatureFilter sensor3Filter;

    TemperatureFilter averageFilter;
    TemperatureFilter hotFilter;
};

#endif