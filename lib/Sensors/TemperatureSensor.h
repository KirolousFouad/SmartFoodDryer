#ifndef TEMPERATURE_MANAGER_H
#define TEMPERATURE_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MAX6675.h>

class TemperatureManager
{
public:

    TemperatureManager(
        uint8_t oneWirePin,
        uint8_t max6675SO,
        uint8_t max6675CS,
        uint8_t max6675SCK
    );

    void begin();
    void update();

    // Individual DS18B20 temperatures
    float getSensor1() const;
    float getSensor2() const;
    float getSensor3() const;

    // Chamber calculations
    float getAverage() const;
    float getMinimum() const;
    float getMaximum() const;

    // MAX6675 temperature
    float getHotTemperature() const;

    // Sensor status
    bool isReady() const;
    bool hasSensorError() const;

private:

    OneWire oneWire;
    DallasTemperature ds18b20;

    MAX6675 thermocouple;

    float sensor1Temperature;
    float sensor2Temperature;
    float sensor3Temperature;

    float averageTemperature;
    float minimumTemperature;
    float maximumTemperature;

    float hotTemperature;

    bool sensorError;
    bool initialized;
};

#endif