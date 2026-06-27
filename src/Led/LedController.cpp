/**
 * @file LedController.cpp
 * @brief Execution routines for low-level pixel register mutations.
 */

#include <FastLED.h> // <- ADICIONE ESTA LINHA AQUI
#include "LedController.h"

LedController g_ledEngine;

void LedController::initHardware() {
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (!g_strips[i].enabled) continue;

        // Alocação dinâmica real do array de pixels baseado na fita do JSON
        g_strips[i].ledBuffer = new CRGB[g_strips[i].ledCount];

        switch (g_strips[i].gpio) {
            case 0:  FastLED.addLeds<WS2812B, 0,  GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 2:  FastLED.addLeds<WS2812B, 2,  GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 4:  FastLED.addLeds<WS2812B, 4,  GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 16: FastLED.addLeds<WS2812B, 16, GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 17: FastLED.addLeds<WS2812B, 17, GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            case 18: FastLED.addLeds<WS2812B, 18, GRB>(g_strips[i].ledBuffer, g_strips[i].ledCount).setCorrection(TypicalLEDStrip); break;
            default: break;
        }
    }
    clearAll();
    show();
}

void LedController::setPixel(uint8_t stripIdx, uint16_t ledIdx, CRGB color) {
    if (stripIdx < CONFIG_MAX_STRIPS && g_strips[stripIdx].enabled) {
        if (ledIdx < g_strips[stripIdx].ledCount) {
            g_strips[stripIdx].ledBuffer[ledIdx] = color;
        }
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
    uint8_t outputBrightness = (g_cfg.brightness * g_cfg.maxBrightness) / 255;
    FastLED.setBrightness(outputBrightness);
}