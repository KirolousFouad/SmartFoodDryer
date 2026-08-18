#include "DataLogger.h"
#include <stdio.h>

DataLogger::DataLogger(uint8_t sdCsPin,
                       uint8_t rtcClkPin,
                       uint8_t rtcDatPin,
                       uint8_t rtcRstPin)
    : sdCsPin(sdCsPin),
      rtcManager(rtcClkPin, rtcDatPin, rtcRstPin),
      sdReady(false),
      sessionActive(false)
{
    sessionFileName[0] = '\0';
}

bool DataLogger::begin()
{
    rtcManager.begin();

    Serial.println("[LOGGER] Initializing SD card...");

    if (!SD.begin(sdCsPin))
    {
        Serial.println("[LOGGER] ERROR: SD card initialization failed");
        sdReady = false;
        return false;
    }

    sdReady = true;

    RTCDateTime now;
    if (!rtcManager.read(now))
    {
        Serial.println("[LOGGER] WARNING: RTC time is invalid");
    }
    else
    {
        Serial.print("[LOGGER] RTC: ");
        Serial.print(now.year);
        Serial.print('-');
        Serial.print(now.month);
        Serial.print('-');
        Serial.print(now.day);
        Serial.print(' ');
        Serial.print(now.hour);
        Serial.print(':');
        Serial.print(now.minute);
        Serial.print(':');
        Serial.println(now.second);
    }

    Serial.println("[LOGGER] Data logger ready");
    return true;
}

bool DataLogger::isReady() const
{
    return sdReady;
}

bool DataLogger::makeSessionFileName()
{
    RTCDateTime now;

    if (!rtcManager.read(now))
    {
        snprintf(sessionFileName, sizeof(sessionFileName), "DRYING.CSV");
        return true;
    }

    // 8.3-compatible filename for the Mega SD library.
    // Example: D26081801.CSV
    for (uint8_t index = 1; index <= 99; ++index)
    {
        snprintf(sessionFileName,
                 sizeof(sessionFileName),
                 "D%02u%02u%02u%02u.CSV",
                 (unsigned int)(now.year % 100),
                 (unsigned int)now.month,
                 (unsigned int)now.day,
                 (unsigned int)index);

        if (!SD.exists(sessionFileName))
        {
            return true;
        }
    }

    return false;
}

void DataLogger::printDateTime(File &file, const RTCDateTime &dt)
{
    file.print(dt.year);
    file.print('-');
    if (dt.month < 10) file.print('0');
    file.print(dt.month);
    file.print('-');
    if (dt.day < 10) file.print('0');
    file.print(dt.day);
    file.print(',');

    if (dt.hour < 10) file.print('0');
    file.print(dt.hour);
    file.print(':');
    if (dt.minute < 10) file.print('0');
    file.print(dt.minute);
    file.print(':');
    if (dt.second < 10) file.print('0');
    file.print(dt.second);
}

bool DataLogger::writeHeader()
{
    File file = SD.open(sessionFileName, FILE_WRITE);
    if (!file)
    {
        return false;
    }

    file.println("Date,Time,Record,State,Mode,Recipe,Temperature,TargetTemperature,Weight,TargetWeight,SCRPower");
    file.close();
    return true;
}

bool DataLogger::startSession(bool autoMode,
                              const char *recipeName,
                              float targetTemperature,
                              float startingWeight,
                              float targetWeight)
{
    if (!sdReady)
    {
        return false;
    }

    if (!makeSessionFileName() || !writeHeader())
    {
        Serial.println("[LOGGER] ERROR: Cannot create session file");
        return false;
    }

    sessionActive = true;

    return logEvent("START",
                    "RUNNING",
                    autoMode,
                    recipeName,
                    0.0f,
                    startingWeight,
                    targetWeight,
                    0);
}

bool DataLogger::logSample(const char *stateName,
                           bool autoMode,
                           const char *recipeName,
                           float temperature,
                           float targetTemperature,
                           float weight,
                           float targetWeight,
                           uint8_t scrPower)
{
    if (!sdReady || !sessionActive)
    {
        return false;
    }

    RTCDateTime now;
    if (!rtcManager.read(now))
    {
        return false;
    }

    File file = SD.open(sessionFileName, FILE_WRITE);
    if (!file)
    {
        return false;
    }

    printDateTime(file, now);
    file.print(",DATA,");
    file.print(stateName ? stateName : "UNKNOWN");
    file.print(',');
    file.print(autoMode ? "AUTO," : "MANUAL,");
    file.print(recipeName ? recipeName : "None");
    file.print(',');
    file.print(temperature, 1);
    file.print(',');
    file.print(targetTemperature, 1);
    file.print(',');
    file.print(weight, 1);
    file.print(',');
    file.print(targetWeight, 1);
    file.print(',');
    file.println(scrPower);

    file.close();
    return true;
}

bool DataLogger::logEvent(const char *eventName,
                          const char *stateName,
                          bool autoMode,
                          const char *recipeName,
                          float temperature,
                          float weight,
                          float targetWeight,
                          uint8_t scrPower)
{
    if (!sdReady || !sessionActive)
    {
        return false;
    }

    RTCDateTime now;
    if (!rtcManager.read(now))
    {
        return false;
    }

    File file = SD.open(sessionFileName, FILE_WRITE);
    if (!file)
    {
        return false;
    }

    printDateTime(file, now);
    file.print(",EVENT,");
    file.print(eventName ? eventName : "UNKNOWN");
    file.print(',');
    file.print(autoMode ? "AUTO," : "MANUAL,");
    file.print(recipeName ? recipeName : "None");
    file.print(',');
    file.print(temperature, 1);
    file.print(',');
    file.print(targetWeight, 1);
    file.print(',');
    file.print(weight, 1);
    file.print(',');
    file.print(targetWeight, 1);
    file.print(',');
    file.println(scrPower);

    file.close();
    return true;
}

void DataLogger::finishSession()
{
    if (!sessionActive)
    {
        return;
    }

    sessionActive = false;
    Serial.println("[LOGGER] Session closed");
}

RTCManager &DataLogger::rtc()
{
    return rtcManager;
}
