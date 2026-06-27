/**
 * @file main.cpp
 * @brief Device boot orchestration and execution thread scheduler.
 */

#include <Arduino.h>
#include "Config/Config.h"
#include "Led/LedController.h"
#include "Animation/AnimationEngine.h"

unsigned long g_benchExecutionTimer = 0;
uint8_t g_benchTestPhaseCounter = 0;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n========================================");
    Serial.println("        RIPADO LIGHT CORE SYSTEM OS     ");
    Serial.printf("        Booting Firmware Version: %s\n", RIPADO_VERSION);
    Serial.println("========================================");

    // Boot execution timeline sequence
    Config_LoadDefaultHardware();
    g_ledEngine.initHardware();
    g_ledEngine.applyBrightnessSafety();
    Animation_Init();

    g_benchExecutionTimer = millis();
    Serial.println("[SYS_OS] Base kernel initialization successful. Mainframe online.");
}

void loop() {
    // Continuous background machine processing loop (Non-blocking FSM worker)
    Animation_Update();

    // Bench Automation Testing Pipeline for Core Validation
    unsigned long currentMilis = millis();
    
    // Phase 1: After 3s, trigger the Upward Wave
    if (g_benchTestPhaseCounter == 0 && (currentMilis - g_benchExecutionTimer >= 3000)) {
        g_benchTestPhaseCounter = 1;
        Serial.println("[TEST] Triggering automated upward opening wave (Floor -> Ceiling)...");
        Animation_StartOpening();
    }
    
    // Phase 2: After 10s, trigger a color/brightness change dynamic shift
    if (g_benchTestPhaseCounter == 1 && (currentMilis - g_benchExecutionTimer >= 10000)) {
        g_benchTestPhaseCounter = 2;
        Serial.println("[TEST] Triggering hot color change to Emerald Success Green (#4CAF50)...");
        Animation_StartColorChange(0x4CAF50, 255);
    }

    // Phase 3: After 17s, trigger the Downward Erase Wave
    if (g_benchTestPhaseCounter == 2 && (currentMilis - g_benchExecutionTimer >= 17000)) {
        g_benchTestPhaseCounter = 3;
        Serial.println("[TEST] Triggering downward closing wave (Ceiling -> Floor)...");
        Animation_StartClosing();
    }

    g_ledEngine.show();
    delay(1); // Critical context yield to prevent FreeRTOS core starvation
}