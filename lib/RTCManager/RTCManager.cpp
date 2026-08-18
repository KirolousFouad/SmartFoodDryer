#include "RTCManager.h"

RTCManager::RTCManager(uint8_t clkPin, uint8_t datPin, uint8_t rstPin)
    : clkPin(clkPin), datPin(datPin), rstPin(rstPin)
{
}

void RTCManager::begin()
{
    pinMode(clkPin, OUTPUT);
    pinMode(rstPin, OUTPUT);
    pinMode(datPin, OUTPUT);

    digitalWrite(clkPin, LOW);
    digitalWrite(rstPin, LOW);
    digitalWrite(datPin, LOW);

    // Clear the clock-halt bit without changing the time.
    uint8_t seconds = readRegister(0x81);
    if (seconds & 0x80)
    {
        writeRegister(0x80, seconds & 0x7F);
    }
}

uint8_t RTCManager::decToBcd(uint8_t value)
{
    return ((value / 10) << 4) | (value % 10);
}

uint8_t RTCManager::bcdToDec(uint8_t value)
{
    return ((value >> 4) * 10) + (value & 0x0F);
}

void RTCManager::writeByte(uint8_t value)
{
    pinMode(datPin, OUTPUT);

    for (uint8_t i = 0; i < 8; ++i)
    {
        digitalWrite(datPin, value & 0x01);
        digitalWrite(clkPin, HIGH);
        digitalWrite(clkPin, LOW);
        value >>= 1;
    }
}

uint8_t RTCManager::readByte()
{
    uint8_t value = 0;

    pinMode(datPin, INPUT_PULLUP);

    for (uint8_t i = 0; i < 8; ++i)
    {
        value >>= 1;

        if (digitalRead(datPin))
        {
            value |= 0x80;
        }

        digitalWrite(clkPin, HIGH);
        digitalWrite(clkPin, LOW);
    }

    return value;
}

void RTCManager::writeRegister(uint8_t address, uint8_t value)
{
    digitalWrite(rstPin, HIGH);
    writeByte(address);
    writeByte(value);
    digitalWrite(rstPin, LOW);
}

uint8_t RTCManager::readRegister(uint8_t address)
{
    digitalWrite(rstPin, HIGH);
    writeByte(address | 0x01);
    uint8_t value = readByte();
    digitalWrite(rstPin, LOW);

    pinMode(datPin, OUTPUT);
    digitalWrite(datPin, LOW);

    return value;
}

bool RTCManager::read(RTCDateTime &dateTime)
{
    dateTime.second = bcdToDec(readRegister(0x81) & 0x7F);
    dateTime.minute = bcdToDec(readRegister(0x83) & 0x7F);
    dateTime.hour = bcdToDec(readRegister(0x85) & 0x3F);
    dateTime.day = bcdToDec(readRegister(0x87) & 0x3F);
    dateTime.month = bcdToDec(readRegister(0x89) & 0x1F);
    dateTime.year = 2000 + bcdToDec(readRegister(0x8D));

    return dateTime.month >= 1 && dateTime.month <= 12 &&
           dateTime.day >= 1 && dateTime.day <= 31 &&
           dateTime.hour <= 23 && dateTime.minute <= 59 &&
           dateTime.second <= 59;
}

bool RTCManager::set(const RTCDateTime &dateTime)
{
    if (dateTime.year < 2000 || dateTime.year > 2099 ||
        dateTime.month < 1 || dateTime.month > 12 ||
        dateTime.day < 1 || dateTime.day > 31 ||
        dateTime.hour > 23 || dateTime.minute > 59 ||
        dateTime.second > 59)
    {
        return false;
    }

    // Disable write protection.
    writeRegister(0x8E, 0x00);

    writeRegister(0x80, decToBcd(dateTime.second) & 0x7F);
    writeRegister(0x82, decToBcd(dateTime.minute));
    writeRegister(0x84, decToBcd(dateTime.hour));
    writeRegister(0x86, decToBcd(dateTime.day));
    writeRegister(0x88, decToBcd(dateTime.month));
    writeRegister(0x8A, decToBcd(1));
    writeRegister(0x8C, decToBcd(dateTime.year % 100));
    writeRegister(0x8E, 0x80);

    return true;
}

bool RTCManager::isRunning()
{
    return (readRegister(0x81) & 0x80) == 0;
}

void RTCManager::startClock()
{
    uint8_t seconds = readRegister(0x81);
    writeRegister(0x80, seconds & 0x7F);
}
