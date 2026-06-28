#include "Application.h"

#include <Arduino.h>

#include "../Storage/StorageManager.h"
#include "../Animation/AnimationEngine.h"
#include "../Led/LedController.h"
#include "../Web/WebService.h"
#include "../Integration/CommandManager.h"
#include "../Commissioning/CommissioningManager.h"

Application g_application;

void Application::begin()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("=======================================");
    Serial.println(" RIPADO - ESP32 Architectural LED");
    Serial.println("=======================================");

    initializeStorage();
    initializeHardware();
    initializeNetwork();

    g_commandManager.begin();
    g_commissioning.begin();

    Serial.println("[SYSTEM] Initialization completed.");
}

void Application::update()
{
    g_commandManager.update();

    Animation_Update();

    g_commissioning.update();

    g_ledEngine.show();

    g_webService.update();
}

void Application::initializeStorage()
{
    g_storage.begin();

    Config_LoadDefaultHardware();

    g_storage.loadConfiguration();
}

void Application::initializeHardware()
{
    g_ledEngine.initHardware();

    Animation_Init();
}

void Application::initializeNetwork()
{
    g_webService.initNetworkAndServer();
}