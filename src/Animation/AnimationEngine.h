/**
 * @file AnimationEngine.h
 * @brief Core FSM and geometric synchronization declarations.
 */

#pragma once
#include <Arduino.h>
#include <FastLED.h> // <- Garante a injeção do tipo CRGB globalmente
#include "../Config/Config.h"
#include "../Led/LedController.h"

// Enumeração explícita dos estados da FSM para sanar erros de escopo
enum SystemState {
    STATE_OFF,
    STATE_POWERING_ON,
    STATE_ON,
    STATE_POWERING_OFF,
    STATE_CHANGING_COLOR
};

extern SystemState g_currentState;
extern int16_t g_animationStep;
extern uint16_t g_maxSystemLeds;

void Animation_Init();
void Animation_Update();
void Animation_StartOpening();
void Animation_StartClosing();
void Animation_StartColorChange(uint32_t targetColorHex, uint8_t targetBrightness);