/**
 * @file AnimationEngine.cpp
 * @brief Finite State Machine handling non-blocking geometric cascade effects.
 */

#include "AnimationEngine.h"
#include "Config/Config.h"
#include "Led/LedController.h"

// Internal registers for time tracking and step sequencing
int16_t g_animationStep = 0;
unsigned long g_lastStepTimestamp = 0;

// Registers to hold transitions during a color change cycle
uint32_t g_queuedColorHex = 0xFFFFFF;
uint8_t g_queuedBrightness = 255;

void Animation_Init() {
    g_lastStepTimestamp = millis();
    g_currentState = STATE_OFF;
}

void Animation_StartOpening() {
    g_animationStep = 0;
    g_currentState = STATE_POWERING_ON;
    g_cfg.power = true;
}

void Animation_StartClosing() {
    g_animationStep = g_maxSystemLeds - 1;
    g_currentState = STATE_POWERING_OFF;
    g_cfg.power = false;
}

void Animation_StartColorChange(uint32_t newColorHex, uint8_t newBrightness) {
    g_queuedColorHex = newColorHex;
    g_queuedBrightness = newBrightness;
    g_animationStep = g_maxSystemLeds - 1;
    g_currentState = STATE_CHANGING_COLOR;
}

/**
 * @brief FSM non-blocking worker loop. Evaluates state variables at every CPU cycle.
 */
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
                        
                        // Linear shift compensation based on computed offset
                        int16_t targetLedIdx = g_animationStep - g_strips[i].offset;
                        if (targetLedIdx >= 0 && targetLedIdx < g_strips[i].ledCount) {
                            g_ledEngine.setPixel(i, targetLedIdx, g_cfg.colorHex);
                        }
                    }
                    g_animationStep++;
                } else {
                    g_currentState = STATE_ON; // Stable ON state reached
                }
                break;

            case STATE_POWERING_OFF:
                if (g_animationStep >= 0) {
                    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++) {
                        if (!g_strips[i].enabled) continue;
                        
                        // Downward collapse: strips start erasing instantly from their unique ceiling index
                        int16_t targetLedIdx = g_strips[i].ledCount - 1 - (g_maxSystemLeds - 1 - g_animationStep);
                        if (targetLedIdx >= 0 && targetLedIdx < g_strips[i].ledCount) {
                            g_ledEngine.setPixel(i, targetLedIdx, CRGB::Black);
                        }
                    }
                    g_animationStep--;
                } else {
                    g_currentState = STATE_OFF; // Stable dark system achieved
                }
                break;

            case STATE_CHANGING_COLOR:
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
                    // Safe injection junction: update values when arrays are verified black
                    g_cfg.colorHex = g_queuedColorHex;
                    g_cfg.brightness = g_queuedBrightness;
                    g_ledEngine.applyBrightnessSafety();
                    
                    // Reset steps and drive FSM back upwards with the newly injected profile
                    g_animationStep = 0;
                    g_currentState = STATE_POWERING_ON;
                }
                break;

            default:
                break;
        }
    }
}