/*
 * Mock EEPROM.h for unit testing canfix.cpp outside of the Arduino environment.
 * Simulates a 512-byte EEPROM with read/write and the bitRead/bitSet/bitClear macros.
 */
#pragma once
#include <cstdint>
#include <cstring>

#define EEPROM_SIZE 512

struct EEPROMClass {
    uint8_t data[EEPROM_SIZE];

    EEPROMClass() { memset(data, 0xFF, sizeof(data)); }

    uint8_t read(int addr)             { return data[addr]; }
    void    write(int addr, uint8_t v) { data[addr] = v; }

    void reset() { memset(data, 0xFF, sizeof(data)); }
};

extern EEPROMClass EEPROM;

// Arduino bit manipulation macros
#define bitRead(value, bit)       (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)        ((value) |=  (1UL << (bit)))
#define bitClear(value, bit)      ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bv)  ((bv) ? bitSet(value, bit) : bitClear(value, bit))
