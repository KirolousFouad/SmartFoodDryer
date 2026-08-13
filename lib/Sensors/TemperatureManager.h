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

    // =====================================================
    // DS18B20
    // =====================================================

    float getSensor1Temperature() const;
    float getSensor2Temperature() const;
    float getSensor3Temperature() const;

    // =====================================================
    // AVERAGE
    // =====================================================

    float getAverageTemperature() const;

    // =====================================================
    // MAX6675
    // =====================================================

    float getHotTemperature() const;

    // =====================================================
    // STATUS
    // =====================================================

    uint8_t getSensorCount() const;

    bool hasTemperatureError() const;

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

    // =====================================================
    // ERROR HANDLING
    // =====================================================

    uint8_t temperatureErrorCount;

    bool temperatureError;

    static constexpr uint8_t MAX_TEMPERATURE_ERRORS = 3;
};

#endif