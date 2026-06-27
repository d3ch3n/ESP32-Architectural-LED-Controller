/**
 * @file Config.h
 * @brief Global data structures and Finite State Machine (FSM) indicators.
 * @author Tech Lead - Ripado OS
 * @version 0.1.0-alpha
 */

#pragma once
#include <Arduino.h>
#include <FastLED.h>

#define RIPADO_VERSION "0.1.0-alpha"
#define CONFIG_MAX_STRIPS 3
#define HARDWARE_MAX_LEDS 300

/**
 * @brief Deterministic states of the global animation machine.
 */
enum SystemState {
    STATE_OFF,               // System completely dark
    STATE_POWERING_ON,       // Upward opening cascade animation active (Floor -> Ceiling)
    STATE_ON,                // All pixels stable at target color/brightness
    STATE_CHANGING_COLOR,    // Downward recoil cascade active before swapping color profile
    STATE_POWERING_OFF       // Downward closing cascade animation active (Ceiling -> Floor)
};

/**
 * @brief Object model encapsulating a unique physical LED strip.
 */
struct LedStrip {
    uint8_t  gpio;           // Physical ESP32 data pin
    uint16_t ledCount;       // Total active pixels in this specific strip
    uint16_t offset;         // Automatic startup delay calculated for ceiling synchronization
    CRGB* ledBuffer;      // Dynamic Memory allocation pointer on Heap
    bool     enabled;        // Channel health state flag
};

/**
 * @brief Product operational configuration parameters.
 */
struct SystemSettings {
    bool     power;
    char     profileName[32];     // Name assigned to the installation profile
    char     deviceName[32];      // Discovery identity for smart eco-systems
    uint8_t  brightness;          // Current active brightness (0-255)
    uint8_t  maxBrightness;       // Hardware current suppression safety limit
    uint32_t colorHex;            // Active 24-bit RGB Color Hex
    uint8_t  animationSpeed;      // Pulse step time in milliseconds
};

// Global cross-module interface declarations
extern LedStrip g_strips[CONFIG_MAX_STRIPS];
extern SystemSettings g_cfg;
extern SystemState g_currentState;
extern uint16_t g_maxSystemLeds;

/**
 * @brief Analyzes strip lengths and automatically calculates startup offsets
 * to synchronize wave arrival at the ceiling.
 */
void Config_CalculateGeometry();

/**
 * @brief Injects default factory runtime configurations into the memory heap.
 */
void Config_LoadDefaultHardware();