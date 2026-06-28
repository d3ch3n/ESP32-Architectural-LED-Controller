/**
 * @file StorageManager.h
 * @brief EEPROM/Flash non-volatile storage abstraction layer.
 */

#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../Config/Config.h"

class StorageManager {
public:
    bool begin();
    bool loadConfiguration();
    bool saveConfiguration();
};

extern StorageManager g_storage;

#endif // STORAGEMANAGER_H