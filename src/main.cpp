/**
 * @file main.cpp
 * @brief Device boot orchestration and execution thread scheduler.
 */

#include <Arduino.h>
#include "Config/Config.h"
#include "Led/LedController.h"
#include "Animation/AnimationEngine.h"
#include "Storage/StorageManager.h" // <- INCLUSÃO DO NOVO MÓDULO

unsigned long g_benchExecutionTimer = 0;
uint8_t g_benchTestPhaseCounter = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n========================================");
    Serial.println("        RIPADO LIGHT CORE SYSTEM OS     ");
    Serial.printf("        Booting Firmware Version: %s\n", RIPADO_VERSION);
    Serial.println("========================================");

    // 1. Mount disk and evaluate system maps
    if (g_storage.initFileSystem()) {
        if (!g_storage.loadConfiguration()) {
            // Fallback load if file configuration parsing aborts
            Config_LoadDefaultHardware();
        }
    } else {
        Config_LoadDefaultHardware();
    }

    // 2. Hardware initialization based on the deployed JSON map
    g_ledEngine.initHardware();
    g_ledEngine.applyBrightnessSafety();
    Animation_Init();

    g_benchExecutionTimer = millis();
    Serial.println("[CORE_OS] Base kernel initialization successful. Mainframe online.");
}

void loop() {
    Animation_Update();

    unsigned long currentMilis = millis();
    if (g_benchTestPhaseCounter == 0 && (currentMilis - g_benchExecutionTimer >= 3000)) {
        g_benchTestPhaseCounter = 1;
        Serial.println("[TEST] Triggering automated upward opening wave (Floor -> Ceiling)...");
        Animation_StartOpening();
    }
    
    if (g_benchTestPhaseCounter == 1 && (currentMilis - g_benchExecutionTimer >= 10000)) {
        g_benchTestPhaseCounter = 2;
        Serial.println("[TEST] Triggering hot color change to Emerald Success Green (#4CAF50)...");
        Animation_StartColorChange(0x4CAF50, 255);
    }

    if (g_benchTestPhaseCounter == 2 && (currentMilis - g_benchExecutionTimer >= 17000)) {
        g_benchTestPhaseCounter = 3;
        Serial.println("[TEST] Triggering downward closing wave (Ceiling -> Floor)...");
        Animation_StartClosing();
    }

    g_ledEngine.show();
    delay(1);
}