/*
 * Global mock object instances — compiled once, linked with all test objects.
 */
#include "mocks/can.h"
#include "mocks/EEPROM.h"

MockCanState g_can;
EEPROMClass  EEPROM;
