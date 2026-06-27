/**
 * @file LedController.cpp
 * @brief Low-level hardware abstractions and memory mapping for FastLED.
 */

#include "LedController.h"

LedController g_ledEngine;

/**
 * @brief Binds arrays dynamically to the hardware pins at runtime.
 */
void LedController::initHardware() {
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (!g_strips[i].enabled) continue;

        // Mapeamento dinâmico expandido para suportar os novos pinos de campo
        switch (g_strips[i].gpio) {
            case 0:  FastLED.addLeds<WS2812B, 0,  GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 2:  FastLED.addLeds<WS2812B, 2,  GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 4:  FastLED.addLeds<WS2812B, 4,  GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 16: FastLED.addLeds<WS2812B, 16, GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 17: FastLED.addLeds<WS2812B, 17, GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 18: FastLED.addLeds<WS2812B, 18, GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            default: 
                Serial.printf("[LED_ENG] Error: GPIO %d mapped is not supported by driver switcher.\n", g_strips[i].gpio);
                break;
        }
    }
    clearAll();
    show();
}

void LedController::setPixel(uint8_t stripIdx, uint16_t ledIdx, CRGB color) {
    if (stripIdx < CONFIG_MAX_STRIPS && g_strips[stripIdx].enabled && ledIdx < g_strips[stripIdx].ledCount) {
        g_strips[stripIdx].ledBuffer[ledIdx] = color;
    }
}

void LedController::clearAll() {
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (g_strips[i].enabled && g_strips[i].ledBuffer != nullptr) {
            memset(g_strips[i].ledBuffer, 0, g_strips[i].ledCount * sizeof(CRGB));
        }
    }
}

void LedController::show() {
    FastLED.show();
}

void LedController::applyBrightnessSafety() {
    uint8_t outputBrightness = g_cfg.brightness;
    if (outputBrightness > g_cfg.maxBrightness) {
        outputBrightness = g_cfg.maxBrightness; // Overrules command to protect hardware power supply
    }
    FastLED.setBrightness(outputBrightness);
}