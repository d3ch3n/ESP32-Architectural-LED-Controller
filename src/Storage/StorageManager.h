/**
 * @file StorageManager.h
 * @brief Thread-safe configuration persistence subsystem using LittleFS and JSON serialization.
 * @author Tech Lead - Ripado OS
 * @version 0.2.0
 */

#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../Config/Config.h"

class StorageManager {
public:
    StorageManager() = default;
    ~StorageManager() = default;

    /**
     * @brief Mounts the LittleFS filesystem partition partition.
     * @return true if mounted successfully, false otherwise.
     */
    bool initFileSystem();

    /**
     * @brief Reads config.json from disk and maps values directly into global memory arrays.
     * @return true if parsed and applied successfully.
     */
    bool loadConfiguration();

    /**
     * @brief Serializes runtime parameters from memory structures and saves config.json to flash disk.
     * @return true if written successfully.
     */
    bool saveConfiguration();
};

extern StorageManager g_storage;