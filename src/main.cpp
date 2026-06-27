/**
 * @file main.cpp
 * @brief Device boot orchestration and execution thread scheduler.
 */

#include <Arduino.h>
#include "Config/Config.h"
#include "Led/LedController.h"
#include "Animation/AnimationEngine.h"
#include "Storage/StorageManager.h"
#include "Web/WebService.h" // <- INCLUSÃO DA INTERFACE DE REDE

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n========================================");
    Serial.println("        RIPADO LIGHT CORE SYSTEM OS     ");
    Serial.printf("        Booting Firmware Version: %s\n", RIPADO_VERSION);
    Serial.println("========================================");

    // 1. Mount disk filesystem and evaluate dynamic parameter maps
    if (g_storage.initFileSystem()) {
        if (!g_storage.loadConfiguration()) {
            Config_LoadDefaultHardware();
        }
    } else {
        Config_LoadDefaultHardware();
    }

    // 2. Instantiate and launch network pipes
    g_webService.initNetworkAndServer();

    // 3. Drive hardware engines using properties loaded from the disk profile
    g_ledEngine.initHardware();
    g_ledEngine.applyBrightnessSafety();
    Animation_Init();

    Serial.println("[CORE_OS] Base kernel initialization successful. Mainframe online.");
}

void loop() {
    // Asynchronous system processing pumps
    Animation_Update();
    g_webService.update(); // <- Garanta esta chamada para limpeza de conexões mortas

    g_ledEngine.show();
    delay(1);
}