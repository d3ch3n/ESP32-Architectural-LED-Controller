/**
 * @file AnimationEngine.cpp
 * @brief FSM state triggers with sequential color-drain logic.
 */

#include "AnimationEngine.h"

SystemState g_currentState = STATE_OFF;
int16_t g_animationStep = 0;
uint16_t g_maxSystemLeds = 160; 
unsigned long g_lastStepTimestamp = 0;

// Variáveis isoladas para controle do fluxo de cor
uint32_t g_targetColorHex = 0x1E88E5; 
bool g_isColorTransitionActive = false; 

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
    g_isColorTransitionActive = false; // Cancela transição pendente se ligar/desligar no meio
    g_animationStep = 0;
    g_cfg.power = true;
    g_currentState = STATE_POWERING_ON;
}

void Animation_StartClosing() {
    g_isColorTransitionActive = false;
    g_animationStep = g_maxSystemLeds - 1;
    g_cfg.power = false;
    g_currentState = STATE_POWERING_OFF;
}

// Intercepta a mudança de cor, guarda o alvo e manda apagar primeiro
void Animation_StartColorChange(uint32_t targetColorHex, uint8_t targetBrightness) {
    g_targetColorHex = targetColorHex;
    g_cfg.brightness = targetBrightness;
    g_ledEngine.applyBrightnessSafety();
    
    // Ativa a flag de transição dupla e joga a FSM para recolher a luz atual
    g_isColorTransitionActive = true;
    g_animationStep = g_maxSystemLeds - 1;
    g_currentState = STATE_POWERING_OFF; 
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
                    g_isColorTransitionActive = false; 
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
                    // SE TERMINOU DE RECOLHER E ERA TROCA DE COR: Inverte para subir!
                    if (g_isColorTransitionActive) {
                        g_cfg.colorHex = g_targetColorHex; // Transfere a nova cor para o buffer oficial
                        g_animationStep = 0;               // Reseta o cursor para a base do chão
                        g_currentState = STATE_POWERING_ON; // Altera o estado da FSM para subir acendendo
                    } else {
                        g_currentState = STATE_OFF;        // Desligamento comum
                    }
                }
                break;

            default:
                break;
        }
    }
}