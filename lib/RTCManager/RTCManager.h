#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>

struct RTCDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

class RTCManager
{
public:
    RTCManager(uint8_t clkPin, uint8_t datPin, uint8_t rstPin);

    void begin();
    bool read(RTCDateTime &dateTime);
    bool set(const RTCDateTime &dateTime);
    bool isRunning();
    void startClock();

private:
    uint8_t clkPin;
    uint8_t datPin;
    uint8_t rstPin;

    static uint8_t decToBcd(uint8_t value);
    static uint8_t bcdToDec(uint8_t value);

    void writeByte(uint8_t value);
    uint8_t readByte();
    void writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegister(uint8_t address);
};

#endif
