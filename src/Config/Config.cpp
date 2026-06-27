/**
 * @file Config.cpp
 * @brief Implementation of global structures and automatic geometric calculation.
 * @author Tech Lead - Ripado OS
 * @version 0.1.0-alpha
 */

#include "Config.h"

// Allocation of global instances defined in the interface contract
LedStrip g_strips[CONFIG_MAX_STRIPS];
SystemSettings g_cfg;
SystemState g_currentState = STATE_OFF;
uint16_t g_maxSystemLeds = 0;

/**
 * @brief Analyzes strip lengths and automatically calculates startup offsets
 * to synchronize wave arrival at the ceiling.
 */
void Config_CalculateGeometry() {
    g_maxSystemLeds = 0;

    // Phase 1: Identify the longest active strip in the architecture
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (g_strips[i].enabled && (g_strips[i].ledCount > g_maxSystemLeds)) {
            g_maxSystemLeds = g_strips[i].ledCount;
        }
    }

    // Phase 2: Compute the linear offset to align the ceiling end points
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (g_strips[i].enabled) {
            g_strips[i].offset = g_maxSystemLeds - g_strips[i].ledCount;
        } else {
            g_strips[i].offset = 0;
        }
    }
}

/**
 * @brief Injects default factory runtime configurations into the memory heap.
 */
void Config_LoadDefaultHardware() {
    // Injects an installation profile programmatically (Default Factory Fallback)
    strcpy(g_cfg.profileName, "Sala Principal");
    strcpy(g_cfg.deviceName, "Ripado Mestre");
    g_cfg.brightness = 255;
    g_cfg.maxBrightness = 180; // Safety ceiling for current suppression
    g_cfg.colorHex = 0x1E88E5;  // Corporate Blue UI Default (#1E88E5)
    g_cfg.animationSpeed = 25;  // 25ms execution step per pixel

    // Hardware mapping matching your physical architecture layout
    g_strips[0] = {16, 180, 0, nullptr, true}; // Channel 1: 180 LEDs (Longest)
    g_strips[1] = {17, 156, 0, nullptr, true}; // Channel 2: 156 LEDs (Intermediate)
    g_strips[2] = {18, 92,  0, nullptr, true}; // Channel 3: 92 LEDs  (Shortest)

    // Secure memory allocation inside the ESP32 dynamic Heap space
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (g_strips[i].enabled) {
            g_strips[i].ledBuffer = new CRGB[g_strips[i].ledCount];
        }
    }

    Config_CalculateGeometry();
}