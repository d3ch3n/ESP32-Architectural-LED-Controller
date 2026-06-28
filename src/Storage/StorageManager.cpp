/**
 * @file StorageManager.cpp
 * @brief EEPROM/Flash non-volatile storage abstraction layer.
 */

#include "StorageManager.h"

StorageManager g_storage;

bool StorageManager::begin()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("[STORAGE] Error mounting LittleFS file system.");
        return false;
    }

    Serial.println("[STORAGE] LittleFS file system mounted successfully.");
    return true;
}

bool StorageManager::loadConfiguration()
{
    if (!LittleFS.exists("/config.json"))
    {
        Serial.println("[STORAGE] Configuration file not found. Using factory defaults.");
        return false;
    }

    File configFile = LittleFS.open("/config.json", "r");

    if (!configFile)
    {
        Serial.println("[STORAGE] Failed to open configuration file.");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);

    configFile.close();

    if (error)
    {
        Serial.println("[STORAGE] Failed to parse configuration JSON.");
        return false;
    }

    g_cfg.power = doc["power"] | g_cfg.power;
    g_cfg.brightness = doc["brightness"] | g_cfg.brightness;
    g_cfg.maxBrightness = doc["maxBrightness"] | g_cfg.maxBrightness;
    g_cfg.colorHex = doc["colorHex"] | g_cfg.colorHex;
    g_cfg.animationSpeed = doc["animationSpeed"] | g_cfg.animationSpeed;

    if (doc["profileName"].is<const char*>())
    {
        strlcpy(
            g_cfg.profileName,
            doc["profileName"],
            sizeof(g_cfg.profileName));
    }

    if (doc["deviceName"].is<const char*>())
    {
        strlcpy(
            g_cfg.deviceName,
            doc["deviceName"],
            sizeof(g_cfg.deviceName));
    }

    if (doc["strips"].is<JsonArray>())
    {
        JsonArray stripsArray = doc["strips"].as<JsonArray>();

        uint8_t index = 0;

        for (JsonObject stripNode : stripsArray)
        {
            if (index >= CONFIG_MAX_STRIPS)
                break;

            g_strips[index].gpio =
                stripNode["gpio"] | g_strips[index].gpio;

            g_strips[index].ledCount =
                stripNode["ledCount"] | g_strips[index].ledCount;

            g_strips[index].offset =
                stripNode["offset"] | g_strips[index].offset;

            g_strips[index].enabled =
                stripNode["enabled"] | g_strips[index].enabled;

            g_strips[index].ledBuffer = nullptr;

            index++;
        }
    }

    Serial.println("[STORAGE] Configuration successfully restored from Flash memory.");
    return true;
}

bool StorageManager::saveConfiguration()
{
    File configFile = LittleFS.open("/config.json", "w");

    if (!configFile)
    {
        Serial.println("[STORAGE] Failed to create configuration file for writing.");
        return false;
    }

    JsonDocument doc;

    doc["power"] = g_cfg.power;
    doc["profileName"] = g_cfg.profileName;
    doc["deviceName"] = g_cfg.deviceName;
    doc["brightness"] = g_cfg.brightness;
    doc["maxBrightness"] = g_cfg.maxBrightness;
    doc["colorHex"] = g_cfg.colorHex;
    doc["animationSpeed"] = g_cfg.animationSpeed;

    JsonArray stripsArray = doc["strips"].to<JsonArray>();

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        JsonObject stripNode = stripsArray.add<JsonObject>();

        stripNode["gpio"] = g_strips[i].gpio;
        stripNode["ledCount"] = g_strips[i].ledCount;
        stripNode["offset"] = g_strips[i].offset;
        stripNode["enabled"] = g_strips[i].enabled;
    }

    if (serializeJsonPretty(doc, configFile) == 0)
    {
        Serial.println("[STORAGE] Failed to write JSON configuration data to flash.");

        configFile.close();

        return false;
    }

    configFile.close();

    Serial.println("[STORAGE] Configuration persisted to flash memory.");
    return true;
}