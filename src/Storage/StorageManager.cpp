/**
 * @file StorageManager.cpp
 * @brief EEPROM/Flash non-volatile storage abstraction layer.
 */

#include "StorageManager.h"

StorageManager g_storage;

bool StorageManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("[STORAGE] Error mounting LittleFS file system.");
        return false;
    }
    Serial.println("[STORAGE] LittleFS file system mounted successfully.");
    return true;
}

bool StorageManager::loadConfiguration() {
    if (!LittleFS.exists("/config.json")) {
        Serial.println("[STORAGE] Configuration file not found. Loading factory defaults.");
        return false;
    }

    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("[STORAGE] Failed to open configuration file.");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Serial.println("[STORAGE] Failed to parse configuration JSON.");
        return false;
    }

    g_cfg.power = doc["power"] | true;
    g_cfg.brightness = doc["brightness"] | 255;
    g_cfg.colorHex = doc["colorHex"] | 0xFFFFFF;
    if (doc["profileName"].is<const char*>()) {
        strncpy(g_cfg.profileName, doc["profileName"], sizeof(g_cfg.profileName) - 1);
    }

    Serial.println("[STORAGE] Configuration successfully restored from Flash memory.");
    return true;
}

bool StorageManager::saveConfiguration() {
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("[STORAGE] Failed to create configuration file for writing.");
        return false;
    }

    JsonDocument doc;
    doc["power"] = g_cfg.power;
    doc["brightness"] = g_cfg.brightness;
    doc["colorHex"] = g_cfg.colorHex;
    doc["profileName"] = g_cfg.profileName;

    if (serializeJson(doc, configFile) == 0) {
        Serial.println("[STORAGE] Failed to write JSON configuration data to flash.");
        configFile.close();
        return false;
    }

    configFile.close();
    Serial.println("[STORAGE] Hardware configuration persisted to system disk.");
    return true;
}