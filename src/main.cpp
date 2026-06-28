/**
 * @file main.cpp
 * @brief Main execution core loop with Alexa and FSM updates.
 */

#include <Arduino.h>
#include <WiFi.h>              // <-- EXPÕE O WI-FI LOCAL PARA TODO O PROJETO
#include <WiFiClientSecure.h>  // <-- EXPÕE A CRIPTOGRAFIA PARA OS WEBSOCKETS
#include <WiFiManager.h>       // <-- INCLUDE INJETADO NO TOPO CORRETAMENTE

#include "Config/Config.h"
#include "Storage/StorageManager.h"
#include "Led/LedController.h"
#include "Animation/AnimationEngine.h"
#include "Web/WebService.h"

void setup() {
    Serial.begin(115200);
    Serial.println("\n========================================");
    Serial.println("        RIPADO LIGHT CORE SYSTEM OS     ");
    Serial.println("========================================");

    g_storage.begin();
    
    if (!g_storage.loadConfiguration()) {
        Config_LoadDefaultHardware();
    }

    // --- JANELA DE RESET DE WI-FI VIA SERIAL ---
    Serial.println("[SYSTEM] Pressione 'r' em ate 3 segundos para resetar o Wi-Fi...");
    unsigned long startCheck = millis();
    while (millis() - startCheck < 3000) {
        if (Serial.available() > 0) {
            char c = Serial.read();
            if (c == 'r' || c == 'R') {
                WiFiManager wm; // Chamada limpa sem o include local
                wm.resetSettings(); // Limpa as credenciais salvas na Flash
                Serial.println("[SYSTEM] Memoria de Wi-Fi limpa com sucesso!");
                break;
            }
        }
    }
    // --------------------------------------------

    g_ledEngine.initHardware();
    Animation_Init();
    g_webService.initNetworkAndServer();
    
    Serial.println("[CORE_OS] Base kernel initialization successful. Mainframe online.");
}

void loop() {
    g_webService.update();
    Animation_Update();
    g_ledEngine.show();
}