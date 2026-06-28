/**
 * @file WebService.h
 * @brief Network abstraction layer and bidirectional payload handler.
 */

#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

#include "../Config/Config.h"

class WebService {
public:
    void initNetworkAndServer();
    void update();
    void broadcastWebSocketMessage(const String& jsonPayload);
};

// Declarações globais externas para o Kernel do sistema
extern WebService g_webService;
extern AsyncWebServer g_server;
extern AsyncWebSocket g_ws;

#endif // WEBSERVICE_H