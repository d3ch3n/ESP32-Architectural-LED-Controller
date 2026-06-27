/**
 * @file StorageManager.cpp
 * @brief Execution routines for memory tree serialization.
 */

#include "StorageManager.h"

StorageManager g_storage;

bool StorageManager::initFileSystem() {
    // Mounts LittleFS, formatting the partition on failure if true is passed
    if (!LittleFS.begin(true)) {
        Serial.println("[STORAGE] Error: Critical failure mounting LittleFS system partition.");
        return false;
    }
    Serial.println("[STORAGE] LittleFS mounted successfully.");
    return true;
}

bool StorageManager::loadConfiguration() {
    if (!LittleFS.exists("/config.json")) {
        Serial.println("[STORAGE] Warning: config.json not found. Falling back to internal defaults.");
        return false;
    }

    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("[STORAGE] Error: Failed to open config.json for reading.");
        return false;
    }

    // Allocate an explicit JSON document buffer on stack matching configuration size boundaries
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close(); // Immediate pointer isolation

    if (error) {
        Serial.printf("[STORAGE] Error: JSON deserialization failure: %s\n", error.c_str());
        return false;
    }

    // --- EXTRACTING JSON MAP INTO SYSTEM VARIABLES ---
    g_cfg.power = false; // Always boot system safe off
    g_cfg.brightness = doc["brightness"] | 255;
    g_cfg.maxBrightness = doc["maxBrightness"] | 180;
    g_cfg.colorHex = doc["colorHex"] | 0x1E88E5;
    g_cfg.animationSpeed = doc["animationSpeed"] | 20;
    
    strlcpy(g_cfg.profileName, doc["profileName"] | "Default Profile", sizeof(g_cfg.profileName));
    strlcpy(g_cfg.deviceName, doc["deviceName"] | "Ripado Core", sizeof(g_cfg.deviceName));

    // --- RE-MAPPING VECTOR HARDWARE OBJECTS ---
    JsonArray stripsArray = doc["strips"];
    uint8_t indexCounter = 0;
    
    for (JsonObject stripNode : stripsArray) {
        if (indexCounter >= CONFIG_MAX_STRIPS) break;

        g_strips[indexCounter].gpio = stripNode["gpio"] | 16;
        g_strips[indexCounter].ledCount = stripNode["ledCount"] | 100;
        g_strips[indexCounter].enabled = stripNode["enabled"] | false;
        
        indexCounter++;
    }

    Serial.printf("[STORAGE] Configuration loaded successfully. Active profile: '%s'\n", g_cfg.profileName);
    return true;
}

bool StorageManager::saveConfiguration() {
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("[STORAGE] Error: Failed to open config.json for writing operations.");
        return false;
    }

    StaticJsonDocument<768> doc;
    doc["profileName"] = g_cfg.profileName;
    doc["deviceName"] = g_cfg.deviceName;
    doc["brightness"] = g_cfg.brightness;
    doc["maxBrightness"] = g_cfg.maxBrightness;
    doc["colorHex"] = g_cfg.colorHex;
    doc["animationSpeed"] = g_cfg.animationSpeed;

    JsonArray stripsArray = doc.createNestedArray("strips");
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        JsonObject stripNode = stripsArray.createNestedObject();
        stripNode["gpio"] = g_strips[i].gpio;
        stripNode["ledCount"] = g_strips[i].ledCount;
        stripNode["enabled"] = g_strips[i].enabled;
    }

    if (serializeJson(doc, configFile) == 0) {
        Serial.println("[STORAGE] Error: Failed to serialize variables into file stream.");
        configFile.close();
        return false;
    }

    configFile.close();
    Serial.println("[STORAGE] Dynamic operational parameters committed to flash successfully.");
    return true;
}