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
#include "../Animation/AnimationEngine.h"

#include <WiFi.h>

WebService g_webService;
AsyncWebServer g_server(80);
AsyncWebSocket g_ws("/ws");

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; 
        JsonDocument doc;
        if (deserializeJson(doc, data)) return;

        if (doc["cmd"].is<JsonVariant>()) {
            String command = doc["cmd"].as<String>();
            if (command == "power") {
                bool state = doc["value"];
                if (state) Animation_StartOpening();
                else Animation_StartClosing();
            }
            if (command == "brightness") {
                uint8_t value = doc["value"];
                Animation_StartColorChange(g_cfg.colorHex, value);
            }
            if (command == "color") {
                uint32_t hexValue = doc["value"];
                Animation_StartColorChange(hexValue, g_cfg.brightness);
            }
        }
    }
}

void onLocalWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            {
                JsonDocument syncDoc;
                syncDoc["type"] = "status";
                syncDoc["power"] = g_cfg.power;
                syncDoc["brightness"] = g_cfg.brightness;
                syncDoc["profileName"] = g_cfg.profileName;
                
                JsonArray stripsArray = syncDoc["strips"].to<JsonArray>();
                for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
                    JsonObject stripNode = stripsArray.add<JsonObject>();
                    stripNode["gpio"] = g_strips[i].gpio;
                    stripNode["ledCount"] = g_strips[i].ledCount;
                }
                
                String responseBuffer;
                serializeJson(syncDoc, responseBuffer);
                client->text(responseBuffer);
            }
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        default:
            break;
    }
}

void WebService::initNetworkAndServer() {
    WiFiManager wm;
    wm.setConnectTimeout(30); 

    if (!wm.autoConnect("Ripado Setup")) {
        delay(1000);
        ESP.restart();
    }

    // Exibe o IP obtido no Monitor Serial para usarmos no painel do eWeLink
    Serial.print(F("[INFO] IP do seu Ripado na rede local: "));
    Serial.println(WiFi.localIP());

    if (MDNS.begin(g_cfg.deviceName)) {
        MDNS.addService("http", "tcp", 80);
    }

    g_ws.onEvent(onLocalWebSocketEvent);
    g_server.addHandler(&g_ws);

    // =========================================================================
    // ENDPOINTS DIRETOS PARA WEBHOOKS DO EWELINK ADVANCED
    // =========================================================================
    
    // 1. Rota de Liga/Desliga: POST para http://IP_DO_ESP32/api/ewelink/power?state=on
    g_server.on("/api/ewelink/power", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("state")) {
            String state = request->getParam("state")->value();
            if (state == "on" || state == "ON") {
                Animation_StartOpening();
                Serial.println(F("[EWELINK] Comando HTTP: LIGAR"));
            } else {
                Animation_StartClosing();
                Serial.println(F("[EWELINK] Comando HTTP: DESLIGAR"));
            }
            request->send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            request->send(400, "application/json", "{\"status\":\"missing_parameter_state\"}");
        }
    });

    // 2. Rota de Brilho: POST para http://IP_DO_ESP32/api/ewelink/brightness?value=80
    g_server.on("/api/ewelink/brightness", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            int brightPct = request->getParam("value")->value().toInt();
            uint8_t targetBrightness = map(brightPct, 0, 100, 0, 255);
            Animation_StartColorChange(g_cfg.colorHex, targetBrightness);
            Serial.printf("[EWELINK] Comando HTTP Brilho: %d%%\n", brightPct);
            request->send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            request->send(400, "application/json", "{\"status\":\"missing_parameter_value\"}");
        }
    });

    // 3. Rota de Cor: POST para http://IP_DO_ESP32/api/ewelink/color?r=255&g=120&b=0
    g_server.on("/api/ewelink/color", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("r") && request->hasParam("g") && request->hasParam("b")) {
            byte r = request->getParam("r")->value().toInt();
            byte g = request->getParam("g")->value().toInt();
            byte b = request->getParam("b")->value().toInt();
            uint32_t targetColor = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
            Animation_StartColorChange(targetColor, g_cfg.brightness);
            Serial.printf("[EWELINK] Comando HTTP Cor: RGB(%d,%d,%d)\n", r, g, b);
            request->send(200, "application/json", "{\"status\":\"success\"}");
        } else {
            request->send(400, "application/json", "{\"status\":\"missing_rgb_parameters\"}");
        }
    });

    g_server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        if (!deserializeJson(doc, data)) {
            JsonArray stripsArray = doc["strips"];
            uint8_t indexCounter = 0;
            for (JsonObject stripNode : stripsArray) {
                if (indexCounter >= CONFIG_MAX_STRIPS) break;
                g_strips[indexCounter].gpio = stripNode["gpio"];
                g_strips[indexCounter].ledCount = stripNode["ledCount"];
                g_strips[indexCounter].enabled = (g_strips[indexCounter].ledCount > 0);
                indexCounter++;
            }
            g_storage.saveConfiguration();
            request->send(200, "application/json", "{\"status\":\"success\"}");
            delay(2000);
            ESP.restart();
        } else {
            request->send(400, "application/json", "{\"status\":\"failed\"}");
        }
    });

    g_server.on("/api/reset_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"wifi_cleared_rebooting\"}");
        delay(1000);
        WiFiManager wm;
        wm.resetSettings();
        ESP.restart();
    });

    g_server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    g_server.begin();
}

void WebService::update() {
    g_ws.cleanupClients();
}

void WebService::broadcastWebSocketMessage(const String& jsonPayload) {
    if (g_ws.count() > 0) {
        g_ws.textAll(jsonPayload);
    }
}