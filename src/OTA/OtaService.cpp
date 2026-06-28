#include "OtaService.h"

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "../Config/Config.h"

OtaService g_otaService;

static char g_otaHostname[64];

const char* OtaService::buildHostname()
{
    String rawName = String(g_cfg.deviceName);

    rawName.trim();
    rawName.toLowerCase();
    rawName.replace(" ", "-");
    rawName.replace("_", "-");

    String cleanName = "";

    for (uint16_t i = 0; i < rawName.length(); i++)
    {
        char c = rawName.charAt(i);

        if ((c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-')
        {
            cleanName += c;
        }
    }

    while (cleanName.startsWith("-"))
        cleanName.remove(0, 1);

    while (cleanName.endsWith("-"))
        cleanName.remove(cleanName.length() - 1, 1);

    if (cleanName.length() == 0)
        cleanName = "ledstrip-pixel";

    strlcpy(g_otaHostname, cleanName.c_str(), sizeof(g_otaHostname));

    return g_otaHostname;
}

void OtaService::begin()
{
    const char* hostname = buildHostname();

    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([]()
    {
        Serial.println("[OTA] Update started.");
    });

    ArduinoOTA.onEnd([]()
    {
        Serial.println();
        Serial.println("[OTA] Update finished.");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        Serial.printf(
            "[OTA] Progress: %u%%\r",
            (progress * 100) / total);
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Serial.printf("[OTA] Error[%u]: ", error);

        if (error == OTA_AUTH_ERROR)
            Serial.println("Auth failed.");
        else if (error == OTA_BEGIN_ERROR)
            Serial.println("Begin failed.");
        else if (error == OTA_CONNECT_ERROR)
            Serial.println("Connect failed.");
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println("Receive failed.");
        else if (error == OTA_END_ERROR)
            Serial.println("End failed.");
        else
            Serial.println("Unknown error.");
    });

    ArduinoOTA.begin();

    Serial.printf("[OTA] Ready: %s.local:3232\n", hostname);
}

void OtaService::update()
{
    ArduinoOTA.handle();
}