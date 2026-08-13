#include "TemperatureManager.h"
#include <math.h>

// =========================================================
// CONSTRUCTOR
// =========================================================

TemperatureManager::TemperatureManager(
    uint8_t oneWirePin,
    uint8_t thermoCLK,
    uint8_t thermoCS,
    uint8_t thermoDO
)
    : oneWire(oneWirePin),
      ds18b20(&oneWire),
      thermocouple(
          thermoCLK,
          thermoCS,
          thermoDO
      ),
      sensorCount(0),
      lastDS18B20Request(0),
      ds18b20ConversionStarted(false),

      sensor1Temperature(NAN),
      sensor2Temperature(NAN),
      sensor3Temperature(NAN),

      averageTemperature(NAN),
      hotTemperature(NAN),

      temperatureErrorCount(0),
      temperatureError(false)
{
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

    ds18b20.setWaitForConversion(false);

    sensorCount =
        ds18b20.getDeviceCount();

    Serial.print(
        "DS18B20 sensors found: "
    );

    Serial.println(
        sensorCount
    );

    // -----------------------------------------------------
    // Start first conversion
    // -----------------------------------------------------

    ds18b20.requestTemperatures();

    lastDS18B20Request =
        millis();

    ds18b20ConversionStarted =
        true;

    // =====================================================
    // MAX6675
    // =====================================================

    float initialHotTemperature =
        thermocouple.readCelsius();

    if (!isnan(initialHotTemperature))
    {
        hotTemperature =
            hotFilter.update(
                initialHotTemperature
            );

        Serial.print(
            "MAX6675 Temperature: "
        );

        Serial.print(
            hotTemperature
        );

        Serial.println(" C");
    }
    else
    {
        Serial.println(
            "MAX6675 Temperature: waiting..."
        );
    }

    // =====================================================
    // ERROR STATE
    // =====================================================

    temperatureErrorCount = 0;

    temperatureError = false;
}

// =========================================================
// UPDATE
// =========================================================

void TemperatureManager::update()
{
    // =====================================================
    // DS18B20
    // =====================================================

    if (ds18b20ConversionStarted)
    {
        /*
         * DS18B20 conversion can take up to
         * approximately 750 ms at 12-bit resolution.
         *
         * Do not read before conversion is complete.
         */

        if (millis() -
            lastDS18B20Request >= 750UL)
        {
            // =================================================
            // READ SENSORS
            // =================================================

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

            // =================================================
            // VALIDATE DS18B20
            // =================================================

            if (rawSensor1 ==
                DEVICE_DISCONNECTED_C)
            {
                rawSensor1 = NAN;
            }

            if (rawSensor2 ==
                DEVICE_DISCONNECTED_C)
            {
                rawSensor2 = NAN;
            }

            if (rawSensor3 ==
                DEVICE_DISCONNECTED_C)
            {
                rawSensor3 = NAN;
            }

            // =================================================
            // FILTER SENSOR 1
            // =================================================

            if (!isnan(rawSensor1))
            {
                sensor1Temperature =
                    sensor1Filter.update(
                        rawSensor1
                    );
            }

            // =================================================
            // FILTER SENSOR 2
            // =================================================

            if (!isnan(rawSensor2))
            {
                sensor2Temperature =
                    sensor2Filter.update(
                        rawSensor2
                    );
            }

            // =================================================
            // FILTER SENSOR 3
            // =================================================

            if (!isnan(rawSensor3))
            {
                sensor3Temperature =
                    sensor3Filter.update(
                        rawSensor3
                    );
            }

            // =================================================
            // CALCULATE AVERAGE
            // =================================================

            float totalTemperature =
                0.0f;

            uint8_t validSensors =
                0;

            if (!isnan(sensor1Temperature))
            {
                totalTemperature +=
                    sensor1Temperature;

                validSensors++;
            }

            if (!isnan(sensor2Temperature))
            {
                totalTemperature +=
                    sensor2Temperature;

                validSensors++;
            }

            if (!isnan(sensor3Temperature))
            {
                totalTemperature +=
                    sensor3Temperature;

                validSensors++;
            }

            if (validSensors > 0)
            {
                float rawAverage =
                    totalTemperature /
                    validSensors;

                averageTemperature =
                    averageFilter.update(
                        rawAverage
                    );
            }

            // =================================================
            // START NEXT CONVERSION
            // =================================================

            ds18b20.requestTemperatures();

            lastDS18B20Request =
                millis();
        }
    }
    else
    {
        // =================================================
        // SAFETY FALLBACK
        // =================================================

        ds18b20.requestTemperatures();

        lastDS18B20Request =
            millis();

        ds18b20ConversionStarted =
            true;
    }

    // =====================================================
    // MAX6675
    // =====================================================

    float rawHotTemperature =
        thermocouple.readCelsius();

    if (!isnan(rawHotTemperature))
    {
        /*
         * Valid reading.
         *
         * Update the filter and clear any temporary
         * communication error.
         */

        hotTemperature =
            hotFilter.update(
                rawHotTemperature
            );

        temperatureErrorCount = 0;

        temperatureError = false;
    }
    else
    {
        /*
         * IMPORTANT:
         *
         * Do NOT immediately declare a temperature error.
         *
         * MAX6675 can occasionally return an invalid reading
         * during communication.
         */

        if (temperatureErrorCount <
            MAX_TEMPERATURE_ERRORS)
        {
            temperatureErrorCount++;
        }

        if (temperatureErrorCount >=
            MAX_TEMPERATURE_ERRORS)
        {
            temperatureError = true;
        }

        /*
         * Keep the previous valid hotTemperature.
         *
         * This prevents the LCD from suddenly displaying
         * "TEMP ERROR" because of one bad reading.
         */
    }

    // =====================================================
    // SERIAL DIAGNOSTICS
    // =====================================================

    Serial.print(
        "DS18B20 Count: "
    );

    Serial.print(
        sensorCount
    );

    Serial.print(
        " | S1: "
    );

    if (isnan(sensor1Temperature))
    {
        Serial.print("--");
    }
    else
    {
        Serial.print(
            sensor1Temperature
        );
    }

    Serial.print(
        " C | S2: "
    );

    if (isnan(sensor2Temperature))
    {
        Serial.print("--");
    }
    else
    {
        Serial.print(
            sensor2Temperature
        );
    }

    Serial.print(
        " C | S3: "
    );

    if (isnan(sensor3Temperature))
    {
        Serial.print("--");
    }
    else
    {
        Serial.print(
            sensor3Temperature
        );
    }

    Serial.print(
        " C | AVG: "
    );

    if (isnan(averageTemperature))
    {
        Serial.print("--");
    }
    else
    {
        Serial.print(
            averageTemperature
        );
    }

    Serial.print(
        " C | HOT: "
    );

    if (isnan(hotTemperature))
    {
        Serial.print("--");
    }
    else
    {
        Serial.print(
            hotTemperature
        );
    }

    Serial.println(
        " C"
    );

    // =====================================================
    // ERROR DEBUG
    // =====================================================

    if (temperatureError)
    {
        Serial.print(
            "[TEMP ERROR] Invalid MAX6675 readings: "
        );

        Serial.println(
            temperatureErrorCount
        );
    }
}

// =========================================================
// GET SENSOR 1
// =========================================================

float TemperatureManager::getSensor1Temperature() const
{
    return sensor1Temperature;
}

// =========================================================
// GET SENSOR 2
// =========================================================

float TemperatureManager::getSensor2Temperature() const
{
    return sensor2Temperature;
}

// =========================================================
// GET SENSOR 3
// =========================================================

float TemperatureManager::getSensor3Temperature() const
{
    return sensor3Temperature;
}

// =========================================================
// GET AVERAGE
// =========================================================

float TemperatureManager::getAverageTemperature() const
{
    return averageTemperature;
}

// =========================================================
// GET HOT TEMPERATURE
// =========================================================

float TemperatureManager::getHotTemperature() const
{
    return hotTemperature;
}

// =========================================================
// GET SENSOR COUNT
// =========================================================

uint8_t TemperatureManager::getSensorCount() const
{
    return sensorCount;
}

// =========================================================
// TEMPERATURE ERROR
// =========================================================

bool TemperatureManager::hasTemperatureError() const
{
    return temperatureError;
}