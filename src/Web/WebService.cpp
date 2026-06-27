/**
 * @file WebService.cpp
 * @brief Network abstraction layer and bidirectional payload handler.
 */

#include "WebService.h" // <- Tudo o que o arquivo precisa já está limpo aqui dentro

WebService g_webService;

// Instantiation of server components on default HTTP port 80 and WebSocket lane /ws
AsyncWebServer g_server(80);
AsyncWebSocket g_ws("/ws");

/**
 * @brief Internal callback to parse inbound WebSocket string frames.
 */
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; // Null-terminate stream packet safely
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        if (error) return;

        // Route commands directly into the isolated AnimationEngine anchors
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

/**
 * @brief Core WebSocket structural asynchronous event distributor.
 */
void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("[WEB] Client integrated. IP: %s | ID: %u\n", client->remoteIP().toString().c_str(), client->id());
            // Force push active memory profile state instantly to mirror switch sliders on connection
            {
                JsonDocument syncDoc;
                syncDoc["type"] = "status";
                syncDoc["power"] = g_cfg.power;
                syncDoc["brightness"] = g_cfg.brightness;
                syncDoc["profileName"] = g_cfg.profileName;
                String responseBuffer;
                serializeJson(syncDoc, responseBuffer);
                client->text(responseBuffer);
            }
            break;
            
        case WS_EVT_DISCONNECT:
            Serial.printf("[WEB] Client disconnected. Connection ID: %u\n", client->id());
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
    wm.setConnectTimeout(30); // 30-second safe threshold boundary

    Serial.println("[WEB] Testing environment connection mappings...");
    
    // Automated Captive Portal instantiation on connection drops
    if (!wm.autoConnect("Ripado Setup")) {
        Serial.println("[WEB] Critical Timeout: Connection dropped. Triggering immediate system restart.");
        delay(1000);
        ESP.restart();
    }

    Serial.printf("[WEB] Network operational. Local IP Assigned: %s\n", WiFi.localIP().toString().c_str());

    // Broadcast mDNS pointer link allowing access via http://ripado.local
    if (MDNS.begin(g_cfg.deviceName)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[WEB] mDNS Resolver established: http://%s.local\n", g_cfg.deviceName);
    }

    // Attach WebSocket architecture events
    g_ws.onEvent(onWebSocketEvent);
    g_server.addHandler(&g_ws);

    // Serve raw static content hosted inside LittleFS directly into target device browsers
    g_server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Start listening on TCP/IP stack lanes
    g_server.begin();
    Serial.println("[WEB] Async Web Server actively listening.");
}

void WebService::update() {
    // Background garbage collection cleanup to purge dead connection frames
    g_ws.cleanupClients();
}

void WebService::broadcastWebSocketMessage(const String& jsonPayload) {
    if (g_ws.count() > 0) {
        g_ws.textAll(jsonPayload); // Forward frame to all client phones synchronously
    }
}