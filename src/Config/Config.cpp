/**
 * @file Config.cpp
 * @brief Global parameter defaults instantiation.
 */

#include <Arduino.h> // <- ADICIONE ESTA LINHA
#include "Config.h"

SystemSettings g_cfg;
StripGeometry g_strips[CONFIG_MAX_STRIPS];

void Config_LoadDefaultHardware() {
    g_cfg.power = false;
    
    strlcpy(g_cfg.profileName, "Sala Principal", sizeof(g_cfg.profileName));
    strlcpy(g_cfg.deviceName, "ripadomestre", sizeof(g_cfg.deviceName));
    
    g_cfg.brightness = 255;
    g_cfg.maxBrightness = 180;
    g_cfg.colorHex = 0x1E88E5;
    g_cfg.animationSpeed = 25;

    g_strips[0].gpio = 4;
    g_strips[0].ledCount = 180;
    g_strips[0].enabled = true;

    g_strips[1].gpio = 16;
    g_strips[1].ledCount = 156;
    g_strips[1].enabled = true;

    g_strips[2].gpio = 2;
    g_strips[2].ledCount = 92;
    g_strips[2].enabled = true;
}