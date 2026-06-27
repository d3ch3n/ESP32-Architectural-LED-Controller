/**
 * @file AnimationEngine.cpp
 * @brief FSM state triggers and non-blocking calculations.
 */

#include "AnimationEngine.h"

// Instanciação explícita das variáveis globais da FSM
SystemState g_currentState = STATE_OFF;
int16_t g_animationStep = 0;
uint16_t g_maxSystemLeds = 180; 
unsigned long g_lastStepTimestamp = 0;

void Animation_Init() {
    g_maxSystemLeds = 0;
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (g_strips[i].enabled && g_strips[i].ledCount > g_maxSystemLeds) {
            g_maxSystemLeds = g_strips[i].ledCount;
        }
    }

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
        if (g_strips[i].enabled) {
            g_strips[i].offset = g_maxSystemLeds - g_strips[i].ledCount;
        }
    }
    g_currentState = STATE_OFF;
}

void Animation_StartOpening() {
    g_animationStep = 0;
    g_cfg.power = true;
    g_currentState = STATE_POWERING_ON;
}

void Animation_StartClosing() {
    g_animationStep = g_maxSystemLeds - 1;
    g_cfg.power = false;
    g_currentState = STATE_POWERING_OFF;
}

void Animation_StartColorChange(uint32_t targetColorHex, uint8_t targetBrightness) {
    g_cfg.colorHex = targetColorHex;
    g_cfg.brightness = targetBrightness;
    g_ledEngine.applyBrightnessSafety();
    g_animationStep = g_maxSystemLeds - 1;
    g_currentState = STATE_CHANGING_COLOR;
}

void Animation_Update() {
    if (g_currentState == STATE_OFF || g_currentState == STATE_ON) return;

    unsigned long currentTimestamp = millis();
    if (currentTimestamp - g_lastStepTimestamp >= g_cfg.animationSpeed) {
        g_lastStepTimestamp = currentTimestamp;

        switch (g_currentState) {
            case STATE_POWERING_ON:
                if (g_animationStep < g_maxSystemLeds) {
                    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
                        if (!g_strips[i].enabled) continue;
                        int16_t targetLedIdx = g_animationStep - g_strips[i].offset;
                        if (targetLedIdx >= 0 && targetLedIdx < g_strips[i].ledCount) {
                            g_ledEngine.setPixel(i, targetLedIdx, CRGB(g_cfg.colorHex));
                        }
                    }
                    g_animationStep++;
                } else {
                    g_currentState = STATE_ON;
                }
                break;

            case STATE_POWERING_OFF:
                if (g_animationStep >= 0) {
                    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
                        if (!g_strips[i].enabled) continue;
                        int16_t targetLedIdx = g_strips[i].ledCount - 1 - (g_maxSystemLeds - 1 - g_animationStep);
                        if (targetLedIdx >= 0 && targetLedIdx < g_strips[i].ledCount) {
                            g_ledEngine.setPixel(i, targetLedIdx, CRGB::Black);
                        }
                    }
                    g_animationStep--;
                } else {
                    g_currentState = STATE_OFF;
                }
                break;

            case STATE_CHANGING_COLOR:
                if (g_animationStep >= 0) {
                    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
                        if (!g_strips[i].enabled) continue;
                        int16_t targetLedIdx = g_strips[i].ledCount - 1 - (g_maxSystemLeds - 1 - g_animationStep);
                        if (targetLedIdx >= 0 && targetLedIdx < g_strips[i].ledCount) {
                            g_ledEngine.setPixel(i, targetLedIdx, CRGB(g_cfg.colorHex));
                        }
                    }
                    g_animationStep--;
                } else {
                    g_currentState = STATE_ON;
                }
                break;
        }
    }
}