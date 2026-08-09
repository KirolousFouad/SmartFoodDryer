#include "TemperatureManager.h"

TemperatureManager::TemperatureManager(
    uint8_t oneWirePin,
    uint8_t thermoCLK,
    uint8_t thermoCS,
    uint8_t thermoDO
)
    : oneWire(oneWirePin),
      ds18b20(&oneWire),
      thermocouple(thermoCLK, thermoCS, thermoDO)
{
    sensor1Temperature = NAN;
    sensor2Temperature = NAN;
    sensor3Temperature = NAN;

    averageTemperature = NAN;

    sensorCount = 0;

    hotTemperature = NAN;
}


// =========================================================
// BEGIN
// =========================================================

void TemperatureManager::begin()
{
    // =====================================================
    // DS18B20
    // =====================================================

    ds18b20.begin();

    sensorCount = ds18b20.getDeviceCount();

    Serial.print("DS18B20 sensors found: ");
    Serial.println(sensorCount);

    delay(500);


    // =====================================================
    // MAX6675
    // =====================================================

    hotTemperature = thermocouple.readCelsius();

    Serial.print("MAX6675 Temperature: ");

    if (isnan(hotTemperature))
    {
        Serial.println("ERROR");
    }
    else
    {
        Serial.print(hotTemperature);
        Serial.println(" C");
    }
}


// =========================================================
// UPDATE
// =========================================================

void TemperatureManager::update()
{
    // =====================================================
    // Request DS18B20 measurements
    // =====================================================

    ds18b20.requestTemperatures();


    // =====================================================
    // Read RAW DS18B20 values
    // =====================================================

    float rawSensor1 = NAN;
    float rawSensor2 = NAN;
    float rawSensor3 = NAN;


    if (sensorCount >= 1)
    {
        rawSensor1 =
            ds18b20.getTempCByIndex(0);
    }

    if (sensorCount >= 2)
    {
        rawSensor2 =
            ds18b20.getTempCByIndex(1);
    }

    if (sensorCount >= 3)
    {
        rawSensor3 =
            ds18b20.getTempCByIndex(2);
    }


    // =====================================================
    // Filter DS18B20 sensors
    // =====================================================

    if (!isnan(rawSensor1))
    {
        sensor1Temperature =
            sensor1Filter.update(rawSensor1);
    }

    if (!isnan(rawSensor2))
    {
        sensor2Temperature =
            sensor2Filter.update(rawSensor2);
    }

    if (!isnan(rawSensor3))
    {
        sensor3Temperature =
            sensor3Filter.update(rawSensor3);
    }


    // =====================================================
    // Calculate average from FILTERED sensors
    // =====================================================

    float totalTemperature = 0.0f;

    uint8_t validSensors = 0;


    if (!isnan(sensor1Temperature))
    {
        totalTemperature += sensor1Temperature;
        validSensors++;
    }

    if (!isnan(sensor2Temperature))
    {
        totalTemperature += sensor2Temperature;
        validSensors++;
    }

    if (!isnan(sensor3Temperature))
    {
        totalTemperature += sensor3Temperature;
        validSensors++;
    }


    if (validSensors > 0)
    {
        float rawAverage =
            totalTemperature / validSensors;

        averageTemperature =
            averageFilter.update(rawAverage);
    }
    else
    {
        averageTemperature = NAN;
    }


    // =====================================================
    // MAX6675
    // =====================================================

    float rawHotTemperature =
        thermocouple.readCelsius();


    // =====================================================
    // Filter MAX6675
    // =====================================================

    if (!isnan(rawHotTemperature))
    {
        hotTemperature =
            hotFilter.update(rawHotTemperature);
    }


    // =====================================================
    // Serial diagnostics
    // =====================================================

    Serial.print("DS18B20 Count: ");
    Serial.print(sensorCount);

    Serial.print(" | S1: ");
    Serial.print(sensor1Temperature);

    Serial.print(" C | S2: ");
    Serial.print(sensor2Temperature);

    Serial.print(" C | S3: ");
    Serial.print(sensor3Temperature);

    Serial.print(" C | AVG: ");
    Serial.print(averageTemperature);

    Serial.print(" C | HOT: ");
    Serial.print(hotTemperature);

    Serial.println(" C");
}


// =========================================================
// GETTERS
// =========================================================

float TemperatureManager::getSensor1Temperature() const
{
    return sensor1Temperature;
}


float TemperatureManager::getSensor2Temperature() const
{
    return sensor2Temperature;
}


float TemperatureManager::getSensor3Temperature() const
{
    return sensor3Temperature;
}


float TemperatureManager::getAverageTemperature() const
{
    return averageTemperature;
}


float TemperatureManager::getHotTemperature() const
{
    return hotTemperature;
}


uint8_t TemperatureManager::getSensorCount() const
{
    return sensorCount;
}