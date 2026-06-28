/**
 * @file WebService.cpp
 * @brief Network abstraction layer, eWeLink Advanced Direct REST API and Local WebSocket handler.
 */

#include <Arduino.h>

#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_DELETE
#undef HTTP_PUT
#undef HTTP_PATCH
#undef HTTP_OPTIONS

#include "WebService.h"

#include "../Storage/StorageManager.h"
#include "../Led/LedController.h"
#include "../Integration/CommandManager.h"
#include "../Commissioning/CommissioningManager.h"

#include <WiFi.h>

WebService g_webService;
AsyncWebServer g_server(80);
AsyncWebSocket g_ws("/ws");

static String buildSafeMdnsName(const char* rawName)
{
    String mdnsName = String(rawName);

    mdnsName.trim();
    mdnsName.toLowerCase();
    mdnsName.replace(" ", "-");
    mdnsName.replace("_", "-");

    String cleanName = "";

    for (uint16_t i = 0; i < mdnsName.length(); i++)
    {
        char c = mdnsName.charAt(i);

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
        cleanName = "ripado";

    return cleanName;
}

static CRGB hexToColor(uint32_t colorHex)
{
    return CRGB(
        (colorHex >> 16) & 0xFF,
        (colorHex >> 8) & 0xFF,
        colorHex & 0xFF);
}

static uint32_t rgbToHex(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) |
           ((uint32_t)g << 8) |
           b;
}

static void sendCurrentStatus(AsyncWebSocketClient* client)
{
    JsonDocument syncDoc;

    syncDoc["type"] = "status";
    syncDoc["power"] = g_commandManager.isOn();
    syncDoc["brightness"] = g_commandManager.brightness();
    syncDoc["profileName"] = g_cfg.profileName;
    syncDoc["deviceName"] = g_cfg.deviceName;
    syncDoc["colorHex"] = g_cfg.colorHex;
    syncDoc["animationSpeed"] = g_cfg.animationSpeed;
    syncDoc["maxBrightness"] = g_cfg.maxBrightness;

    JsonArray stripsArray = syncDoc["strips"].to<JsonArray>();

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        JsonObject stripNode = stripsArray.add<JsonObject>();

        stripNode["gpio"] = g_strips[i].gpio;
        stripNode["ledCount"] = g_strips[i].ledCount;
        stripNode["enabled"] = g_strips[i].enabled;
        stripNode["offset"] = g_strips[i].offset;
    }

    String responseBuffer;
    serializeJson(syncDoc, responseBuffer);

    client->text(responseBuffer);
}

static void broadcastStatus()
{
    if (g_ws.count() == 0)
        return;

    JsonDocument syncDoc;

    syncDoc["type"] = "status";
    syncDoc["power"] = g_commandManager.isOn();
    syncDoc["brightness"] = g_commandManager.brightness();
    syncDoc["profileName"] = g_cfg.profileName;
    syncDoc["deviceName"] = g_cfg.deviceName;
    syncDoc["colorHex"] = g_cfg.colorHex;
    syncDoc["animationSpeed"] = g_cfg.animationSpeed;
    syncDoc["maxBrightness"] = g_cfg.maxBrightness;

    JsonArray stripsArray = syncDoc["strips"].to<JsonArray>();

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        JsonObject stripNode = stripsArray.add<JsonObject>();

        stripNode["gpio"] = g_strips[i].gpio;
        stripNode["ledCount"] = g_strips[i].ledCount;
        stripNode["enabled"] = g_strips[i].enabled;
        stripNode["offset"] = g_strips[i].offset;
    }

    String responseBuffer;
    serializeJson(syncDoc, responseBuffer);

    g_ws.textAll(responseBuffer);
}

static void broadcastCommissioningState(const String& mode)
{
    if (g_ws.count() == 0)
        return;

    JsonDocument doc;

    doc["type"] = "commissioning";
    doc["strip"] = g_commissioning.currentStrip();
    doc["led"] = g_commissioning.currentLed();
    doc["mode"] = mode;
    doc["autoScan"] = g_commissioning.isAutoScanRunning();

    String responseBuffer;
    serializeJson(doc, responseBuffer);

    g_ws.textAll(responseBuffer);
}

static bool runCommissioningCommand(const String& action,
                                    uint8_t strip,
                                    uint16_t led,
                                    uint32_t colorHex,
                                    uint16_t count,
                                    uint16_t intervalMs)
{
    if (strip >= CONFIG_MAX_STRIPS)
        strip = 0;

    CRGB color = hexToColor(colorHex);

    if (action == "single")
    {
        g_commissioning.showSingle(strip, led, color);
        broadcastCommissioningState("single");
        return true;
    }

    if (action == "fill")
    {
        g_commissioning.fillTo(strip, led, color);
        broadcastCommissioningState("fill");
        return true;
    }

    if (action == "off")
    {
        g_commissioning.off();
        broadcastCommissioningState("off");
        return true;
    }

    if (action == "saveCount")
    {
        if (count == 0)
            count = led + 1;

        g_commissioning.saveCount(strip, count);
        broadcastStatus();
        broadcastCommissioningState("saveCount");
        return true;
    }

    if (action == "autoStart")
    {
        g_commissioning.startAutoScan(strip, led, intervalMs, color);
        broadcastCommissioningState("autoStart");
        return true;
    }

    if (action == "autoStop")
    {
        g_commissioning.stopAutoScan();
        broadcastCommissioningState("autoStop");
        return true;
    }

    return false;
}

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len)
{
    AwsFrameInfo* info = (AwsFrameInfo*)arg;

    if (!(info->final &&
          info->index == 0 &&
          info->len == len &&
          info->opcode == WS_TEXT))
    {
        return;
    }

    String message;

    for (size_t i = 0; i < len; i++)
        message += (char)data[i];

    JsonDocument doc;

    if (deserializeJson(doc, message))
        return;

    if (!doc["cmd"].is<JsonVariant>())
        return;

    String command = doc["cmd"].as<String>();

    if (command == "power")
    {
        bool state = doc["value"] | false;

        if (state)
            g_commandManager.powerOn();
        else
            g_commandManager.powerOff();

        broadcastStatus();
        return;
    }

    if (command == "brightness")
    {
        uint8_t value = doc["value"] | g_cfg.brightness;

        g_commandManager.setBrightness(value);

        broadcastStatus();
        return;
    }

    if (command == "color")
    {
        uint32_t hexValue = doc["value"] | g_cfg.colorHex;

        g_commandManager.setColor(hexValue);

        broadcastStatus();
        return;
    }

    if (command == "commissioning")
    {
        String action = doc["action"] | "single";
        uint8_t strip = doc["strip"] | 0;
        uint16_t led = doc["led"] | 0;
        uint32_t colorHex = doc["color"] | 0xFFFFFF;
        uint16_t count = doc["count"] | 0;
        uint16_t intervalMs = doc["interval"] | 120;

        runCommissioningCommand(action, strip, led, colorHex, count, intervalMs);
        return;
    }
}

void onLocalWebSocketEvent(AsyncWebSocket* server,
                           AsyncWebSocketClient* client,
                           AwsEventType type,
                           void* arg,
                           uint8_t* data,
                           size_t len)
{
    switch (type)
    {
        case WS_EVT_CONNECT:
            sendCurrentStatus(client);
            break;

        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;

        default:
            break;
    }
}

void WebService::initNetworkAndServer()
{
    WiFiManager wm;
    wm.setConnectTimeout(30);

    if (!wm.autoConnect("Ripado Setup"))
    {
        delay(1000);
        ESP.restart();
    }

    Serial.print(F("[INFO] IP do seu Ripado na rede local: "));
    Serial.println(WiFi.localIP());

    String mdnsName = buildSafeMdnsName(g_cfg.deviceName);

    if (MDNS.begin(mdnsName.c_str()))
    {
        MDNS.addService("http", "tcp", 80);

        Serial.printf("[MDNS] http://%s.local\n", mdnsName.c_str());
    }
    else
    {
        Serial.println("[MDNS] Failed to start mDNS service.");
    }

    g_ws.onEvent(onLocalWebSocketEvent);
    g_server.addHandler(&g_ws);

    g_server.on("/api/commissioning", HTTP_GET, [](AsyncWebServerRequest* request)
    {
        String action = "single";
        uint8_t strip = 0;
        uint16_t led = 0;
        uint32_t colorHex = 0xFFFFFF;
        uint16_t count = 0;
        uint16_t intervalMs = 120;

        if (request->hasParam("action"))
            action = request->getParam("action")->value();

        if (request->hasParam("strip"))
            strip = request->getParam("strip")->value().toInt();

        if (request->hasParam("led"))
            led = request->getParam("led")->value().toInt();

        if (request->hasParam("color"))
            colorHex = strtoul(request->getParam("color")->value().c_str(), nullptr, 10);

        if (request->hasParam("count"))
            count = request->getParam("count")->value().toInt();

        if (request->hasParam("interval"))
            intervalMs = request->getParam("interval")->value().toInt();

        bool ok = runCommissioningCommand(action, strip, led, colorHex, count, intervalMs);

        if (ok)
        {
            request->send(
                200,
                "application/json",
                "{\"status\":\"success\"}");
        }
        else
        {
            request->send(
                400,
                "application/json",
                "{\"status\":\"invalid_commissioning_command\"}");
        }
    });

    g_server.on("/api/ewelink/power", HTTP_POST, [](AsyncWebServerRequest* request)
    {
        if (!request->hasParam("state"))
        {
            request->send(
                400,
                "application/json",
                "{\"status\":\"missing_parameter_state\"}");

            return;
        }

        String state = request->getParam("state")->value();

        if (state.equalsIgnoreCase("on"))
        {
            g_commandManager.powerOn();
            Serial.println(F("[EWELINK] POWER ON"));
        }
        else
        {
            g_commandManager.powerOff();
            Serial.println(F("[EWELINK] POWER OFF"));
        }

        broadcastStatus();

        request->send(
            200,
            "application/json",
            "{\"status\":\"success\"}");
    });

    g_server.on("/api/ewelink/brightness", HTTP_POST, [](AsyncWebServerRequest* request)
    {
        if (!request->hasParam("value"))
        {
            request->send(
                400,
                "application/json",
                "{\"status\":\"missing_parameter_value\"}");

            return;
        }

        int brightPct = request->getParam("value")->value().toInt();
        brightPct = constrain(brightPct, 0, 100);

        uint8_t targetBrightness = map(brightPct, 0, 100, 0, 255);

        g_commandManager.setBrightness(targetBrightness);

        Serial.printf("[EWELINK] BRIGHTNESS %d%%\n", brightPct);

        broadcastStatus();

        request->send(
            200,
            "application/json",
            "{\"status\":\"success\"}");
    });

    g_server.on("/api/ewelink/color", HTTP_POST, [](AsyncWebServerRequest* request)
    {
        if (!(request->hasParam("r") &&
              request->hasParam("g") &&
              request->hasParam("b")))
        {
            request->send(
                400,
                "application/json",
                "{\"status\":\"missing_rgb_parameters\"}");

            return;
        }

        uint8_t r = constrain(request->getParam("r")->value().toInt(), 0, 255);
        uint8_t g = constrain(request->getParam("g")->value().toInt(), 0, 255);
        uint8_t b = constrain(request->getParam("b")->value().toInt(), 0, 255);

        uint32_t targetColor = rgbToHex(r, g, b);

        g_commandManager.setColor(targetColor);

        Serial.printf("[EWELINK] COLOR RGB(%u,%u,%u)\n", r, g, b);

        broadcastStatus();

        request->send(
            200,
            "application/json",
            "{\"status\":\"success\"}");
    });

    g_server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
        [](AsyncWebServerRequest* request,
           uint8_t* data,
           size_t len,
           size_t index,
           size_t total)
        {
            String body;

            for (size_t i = 0; i < len; i++)
                body += (char)data[i];

            JsonDocument doc;

            if (deserializeJson(doc, body))
            {
                request->send(
                    400,
                    "application/json",
                    "{\"status\":\"failed\"}");

                return;
            }

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

            if (doc["brightness"].is<uint8_t>())
                g_cfg.brightness = doc["brightness"];

            if (doc["maxBrightness"].is<uint8_t>())
                g_cfg.maxBrightness = doc["maxBrightness"];

            if (doc["animationSpeed"].is<uint8_t>())
                g_cfg.animationSpeed = doc["animationSpeed"];

            if (doc["colorHex"].is<uint32_t>())
                g_cfg.colorHex = doc["colorHex"];

            JsonArray stripsArray = doc["strips"];

            uint8_t indexCounter = 0;

            for (JsonObject stripNode : stripsArray)
            {
                if (indexCounter >= CONFIG_MAX_STRIPS)
                    break;

                g_strips[indexCounter].gpio =
                    stripNode["gpio"] | g_strips[indexCounter].gpio;

                g_strips[indexCounter].ledCount =
                    stripNode["ledCount"] | g_strips[indexCounter].ledCount;

                g_strips[indexCounter].enabled =
                    g_strips[indexCounter].ledCount > 0;

                indexCounter++;
            }

            g_storage.saveConfiguration();

            request->send(
                200,
                "application/json",
                "{\"status\":\"success\"}");

            delay(2000);
            ESP.restart();
        });

    g_server.on("/api/reset_wifi", HTTP_GET, [](AsyncWebServerRequest* request)
    {
        request->send(
            200,
            "application/json",
            "{\"status\":\"wifi_cleared_rebooting\"}");

        delay(1000);

        WiFiManager wm;
        wm.resetSettings();

        ESP.restart();
    });

    g_server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    g_server.begin();
}

void WebService::update()
{
    g_ws.cleanupClients();
}

void WebService::broadcastWebSocketMessage(const String& jsonPayload)
{
    if (g_ws.count() > 0)
        g_ws.textAll(jsonPayload);
}