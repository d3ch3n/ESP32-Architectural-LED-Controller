/**
 * @file WebService.cpp
 * @brief Network abstraction layer and bidirectional payload handler.
 */

#include <Arduino.h>
#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_DELETE
#undef HTTP_PUT
#undef HTTP_PATCH
#undef HTTP_OPTIONS

#include "WebService.h"

WebService g_webService;

AsyncWebServer g_server(80);
AsyncWebSocket g_ws("/ws");

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; 
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error) return;

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
            // Roteamento de comandos dentro da função handleWebSocketMessage
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
            
            // -----------------------------------------------------------------
            // NOVO COMANDO: Interceptador do Led Finder de Calibração Visual
            // -----------------------------------------------------------------
            if (command == "finder") {
                uint8_t stripIdx = doc["strip"];
                uint16_t testCount = doc["count"];
                
                // Força o motor gráfico a parar e limpa os buffers de luz
                g_currentState = STATE_OFF; 
                g_ledEngine.clearAll();
                
                // Acende apenas o pixel alvo em Branco Puro (0xFFFFFF) para teste de ponta
                if (testCount > 0) {
                    g_ledEngine.setPixel(stripIdx, testCount - 1, CRGB::White);
                }
                
                // Injeta diretamente no registrador físico sem delay
                g_ledEngine.show();
                Serial.printf("[FINDER] Light pulse injected into Strip [%u] at pixel position: %u\n", stripIdx, testCount - 1);
            }
        }
        }
    }
}

void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            {
                // EXPANSÃO: Envia o status operacional JUNTO com o mapa geométrico das fitas para popular o Installer
                JsonDocument syncDoc;
                syncDoc["type"] = "status";
                syncDoc["power"] = g_cfg.power;
                syncDoc["brightness"] = g_cfg.brightness;
                syncDoc["profileName"] = g_cfg.profileName;
                
                JsonArray stripsArray = syncDoc.createNestedArray("strips");
                for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
                    JsonObject stripNode = stripsArray.createNestedObject();
                    stripNode["gpio"] = g_strips[i].gpio;
                    stripNode["ledCount"] = g_strips[i].ledCount;
                }
                
                String responseBuffer;
                serializeJson(syncDoc, responseBuffer);
                client->text(responseBuffer);
            }
            break;
            
        case WS_EVT_DISCONNECT:
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

    Serial.println("[WEB] Testing environment connection mappings...");
    if (!wm.autoConnect("Ripado Setup")) {
        Serial.println("[WEB] Critical Timeout: Connection dropped. Triggering immediate system restart.");
        delay(1000);
        ESP.restart();
    }

    Serial.printf("[WEB] Network operational. Local IP Assigned: %s\n", WiFi.localIP().toString().c_str());

    if (MDNS.begin(g_cfg.deviceName)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[WEB] mDNS Resolver established: http://%s.local\n", g_cfg.deviceName);
    }

    g_ws.onEvent(onWebSocketEvent);
    g_server.addHandler(&g_ws);

    // -------------------------------------------------------------------------
    // NOVA ROTA REST API: Intercepta e processa o envio das novas pinagens via POST
    // -------------------------------------------------------------------------
    g_server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        
        if (!error) {
            JsonArray stripsArray = doc["strips"];
            uint8_t indexCounter = 0;
            
            for (JsonObject stripNode : stripsArray) {
                if (indexCounter >= CONFIG_MAX_STRIPS) break;

                g_strips[indexCounter].gpio = stripNode["gpio"];
                g_strips[indexCounter].ledCount = stripNode["ledCount"];
                g_strips[indexCounter].enabled = (g_strips[indexCounter].ledCount > 0);
                
                indexCounter++;
            }

            // Commita os novos parâmetros de hardware da memória RAM direto no LittleFS
            g_storage.saveConfiguration();
            
            // Envia cabeçalho HTTP 200 de sucesso de forma assíncrona
            request->send(200, "application/json", "{\"status\":\"success\"}");
            
            // Safe delay para encerramento de conexões antes do Reboot de hardware
            Serial.println("[WEB] Dynamic Remap received. Executing hardware safe reboot sequence...");
            delay(2000);
            ESP.restart();
        } else {
            request->send(400, "application/json", "{\"status\":\"failed\"}");
        }
    });

    g_server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    g_server.begin();
    Serial.println("[WEB] Async Web Server actively listening.");
}

void WebService::update() {
    g_ws.cleanupClients();
}

void WebService::broadcastWebSocketMessage(const String& jsonPayload) {
    if (g_ws.count() > 0) {
        g_ws.textAll(jsonPayload);
    }
}