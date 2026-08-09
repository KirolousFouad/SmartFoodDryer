#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <max6675.h>

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

    float getSensor1Temperature() const;
    float getSensor2Temperature() const;
    float getSensor3Temperature() const;

    float getAverageTemperature() const;

    float getHotTemperature() const;

    uint8_t getSensorCount() const;

private:

    // =====================================================
    // DS18B20
    // =====================================================

    OneWire oneWire;
    DallasTemperature ds18b20;

    float sensor1Temperature;
    float sensor2Temperature;
    float sensor3Temperature;

    float averageTemperature;

    uint8_t sensorCount;


    // =====================================================
    // MAX6675 + K-Type
    // =====================================================

    MAX6675 thermocouple;

    float hotTemperature;


    // =====================================================
    // Temperature Filters
    // =====================================================

    TemperatureFilter sensor1Filter;
    TemperatureFilter sensor2Filter;
    TemperatureFilter sensor3Filter;

    TemperatureFilter averageFilter;
    TemperatureFilter hotFilter;
};

#endif