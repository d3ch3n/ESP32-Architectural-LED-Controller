/**
 * @file Config.h
 * @brief Dynamic operating system definitions and data structure map.
 */

#pragma once
#include <Arduino.h>

#define RIPADO_VERSION "0.1.0-alpha"
#define CONFIG_MAX_STRIPS 3

struct SystemSettings {
    bool     power;
    char     profileName[32];     
    char     deviceName[32];      // <- Garanta que este membro está exatamente aqui
    uint8_t  brightness;          
    uint8_t  maxBrightness;       
    uint32_t colorHex;            
    uint8_t  animationSpeed;      
};

struct StripGeometry {
    uint8_t  gpio;
    uint16_t ledCount;
    uint16_t offset;
    bool     enabled;
    struct CRGB* ledBuffer; // Dynamic structural allocation link
};

extern SystemSettings g_cfg;
extern StripGeometry g_strips[CONFIG_MAX_STRIPS];

void Config_LoadDefaultHardware();