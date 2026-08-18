#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include <SD.h>
#include "RTCManager.h"

class DataLogger
{
public:
    DataLogger(uint8_t sdCsPin,
               uint8_t rtcClkPin,
               uint8_t rtcDatPin,
               uint8_t rtcRstPin);

    bool begin();
    bool isReady() const;

    bool startSession(bool autoMode,
                      const char *recipeName,
                      float targetTemperature,
                      float startingWeight,
                      float targetWeight);

    bool logSample(const char *stateName,
                   bool autoMode,
                   const char *recipeName,
                   float temperature,
                   float targetTemperature,
                   float weight,
                   float targetWeight,
                   uint8_t scrPower);

    bool logEvent(const char *eventName,
                  const char *stateName,
                  bool autoMode,
                  const char *recipeName,
                  float temperature,
                  float weight,
                  float targetWeight,
                  uint8_t scrPower);

    void finishSession();

    RTCManager &rtc();

private:
    uint8_t sdCsPin;
    RTCManager rtcManager;

    bool sdReady;
    bool sessionActive;
    char sessionFileName[32];

    bool makeSessionFileName();
    bool writeHeader();
    static void printDateTime(File &file, const RTCDateTime &dt);
};

#endif
