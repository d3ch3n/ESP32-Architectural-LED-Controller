#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "../Config/Config.h"
#include "../Led/LedController.h"

enum SystemState
{
    STATE_OFF,
    STATE_POWERING_ON,
    STATE_ON,
    STATE_POWERING_OFF,
    STATE_CHANGING_COLOR
};

class AnimationEngine
{
public:
    void begin();
    void update();

    void startOpening();
    void startClosing();
    void startColorChange(uint32_t targetColorHex,
                          uint8_t targetBrightness);

    SystemState getState() const;

private:
    void calculateOffsets();
    void updateOpening();
    void updateClosing();
    void finishClosing();
    void prepareOpening();

    bool isStripActive(uint8_t strip) const;

    SystemState currentState = STATE_OFF;

    int16_t animationStep = 0;
    uint16_t maxSystemLeds = 0;

    unsigned long lastStepTimestamp = 0;

    uint32_t pendingColor = 0x1E88E5;
    bool colorTransitionActive = false;
};

extern AnimationEngine g_animation;

void Animation_Init();
void Animation_Update();
void Animation_StartOpening();
void Animation_StartClosing();
void Animation_StartColorChange(uint32_t targetColorHex,
                                uint8_t targetBrightness);