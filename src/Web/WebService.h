/**
 * @file WebService.h
 * @brief Asynchronous Web Server and WebSocket infrastructure definitions.
 * @author Tech Lead - Ripado OS
 * @version 0.3.0
 */

#pragma once

// 1. PRIMEIRO: Força a limpeza das macros globais para evitar colisão com nghttp e WebServer
#include <Arduino.h>
#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_DELETE
#undef HTTP_PUT
#undef HTTP_PATCH
#undef HTTP_OPTIONS

// 2. AGORA SIM: Carrega as dependências de rede sem risco de curto-circuito
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../Config/Config.h"
#include "../Storage/StorageManager.h"
#include "../Animation/AnimationEngine.h"

class WebService {
public:
    WebService() = default;
    ~WebService() = default;

    void initNetworkAndServer();
    void update();
    void broadcastWebSocketMessage(const String& jsonPayload);
};

extern WebService g_webService;