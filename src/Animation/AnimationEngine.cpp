#include "AnimationEngine.h"

AnimationEngine g_animation;

void AnimationEngine::begin()
{
    calculateOffsets();

    currentState = g_cfg.power ? STATE_ON : STATE_OFF;

    if (g_cfg.power)
    {
        g_ledEngine.clearAll();

        for (uint8_t strip = 0; strip < CONFIG_MAX_STRIPS; strip++)
        {
            if (!isStripActive(strip))
                continue;

            g_ledEngine.fillStrip(strip, CRGB(g_cfg.colorHex));
        }
    }
    else
    {
        g_ledEngine.clearAll();
    }
}

void AnimationEngine::calculateOffsets()
{
    maxSystemLeds = 0;

    for (uint8_t strip = 0; strip < CONFIG_MAX_STRIPS; strip++)
    {
        if (!isStripActive(strip))
            continue;

        uint16_t stripLedCount = g_ledEngine.ledCount(strip);

        if (stripLedCount > maxSystemLeds)
            maxSystemLeds = stripLedCount;
    }

    for (uint8_t strip = 0; strip < CONFIG_MAX_STRIPS; strip++)
    {
        if (!isStripActive(strip))
            continue;

        g_strips[strip].offset = maxSystemLeds - g_ledEngine.ledCount(strip);
    }
}

void AnimationEngine::startOpening()
{
    calculateOffsets();

    if (maxSystemLeds == 0)
        return;

    colorTransitionActive = false;

    g_cfg.power = true;

    prepareOpening();

    currentState = STATE_POWERING_ON;
}

void AnimationEngine::startClosing()
{
    calculateOffsets();

    if (maxSystemLeds == 0)
        return;

    colorTransitionActive = false;

    g_cfg.power = false;

    animationStep = 0;
    lastStepTimestamp = 0;

    currentState = STATE_POWERING_OFF;
}

void AnimationEngine::startColorChange(uint32_t targetColorHex,
                                       uint8_t targetBrightness)
{
    calculateOffsets();

    if (maxSystemLeds == 0)
    {
        g_cfg.colorHex = targetColorHex;
        g_cfg.brightness = targetBrightness;
        g_ledEngine.applyBrightnessSafety();
        return;
    }

    pendingColor = targetColorHex;

    g_cfg.brightness = targetBrightness;
    g_ledEngine.applyBrightnessSafety();

    colorTransitionActive = true;

    animationStep = 0;
    lastStepTimestamp = 0;

    currentState = STATE_POWERING_OFF;
}

void AnimationEngine::update()
{
    if (currentState == STATE_OFF ||
        currentState == STATE_ON)
    {
        return;
    }

    unsigned long now = millis();

    if (now - lastStepTimestamp < g_cfg.animationSpeed)
        return;

    lastStepTimestamp = now;

    switch (currentState)
    {
        case STATE_POWERING_ON:
            updateOpening();
            break;

        case STATE_POWERING_OFF:
            updateClosing();
            break;

        default:
            break;
    }
}

void AnimationEngine::updateOpening()
{
    if (animationStep >= maxSystemLeds)
    {
        currentState = STATE_ON;
        colorTransitionActive = false;
        return;
    }

    for (uint8_t strip = 0; strip < CONFIG_MAX_STRIPS; strip++)
    {
        if (!isStripActive(strip))
            continue;

        int16_t physicalLed = animationStep - g_strips[strip].offset;

        if (physicalLed < 0)
            continue;

        if (physicalLed >= g_ledEngine.ledCount(strip))
            continue;

        g_ledEngine.setPixel(
            strip,
            physicalLed,
            CRGB(g_cfg.colorHex));
    }

    animationStep++;
}

void AnimationEngine::updateClosing()
{
    bool anyPixelCleared = false;

    for (uint8_t strip = 0; strip < CONFIG_MAX_STRIPS; strip++)
    {
        if (!isStripActive(strip))
            continue;

        uint16_t stripLedCount = g_ledEngine.ledCount(strip);

        if (animationStep >= stripLedCount)
            continue;

        int16_t physicalLed = stripLedCount - 1 - animationStep;

        if (physicalLed < 0)
            continue;

        g_ledEngine.setPixel(
            strip,
            physicalLed,
            CRGB::Black);

        anyPixelCleared = true;
    }

    animationStep++;

    if (!anyPixelCleared || animationStep > maxSystemLeds)
        finishClosing();
}

void AnimationEngine::finishClosing()
{
    g_ledEngine.clearAll();

    if (colorTransitionActive)
    {
        g_cfg.colorHex = pendingColor;
        g_cfg.power = true;

        prepareOpening();

        currentState = STATE_POWERING_ON;
        return;
    }

    g_cfg.power = false;
    currentState = STATE_OFF;
}

void AnimationEngine::prepareOpening()
{
    g_ledEngine.clearAll();

    animationStep = 0;
    lastStepTimestamp = 0;
}

bool AnimationEngine::isStripActive(uint8_t strip) const
{
    return strip < CONFIG_MAX_STRIPS &&
           g_ledEngine.stripEnabled(strip) &&
           g_ledEngine.ledCount(strip) > 0;
}

SystemState AnimationEngine::getState() const
{
    return currentState;
}

void Animation_Init()
{
    g_animation.begin();
}

void Animation_Update()
{
    g_animation.update();
}

void Animation_StartOpening()
{
    g_animation.startOpening();
}

void Animation_StartClosing()
{
    g_animation.startClosing();
}

void Animation_StartColorChange(uint32_t color,
                                uint8_t brightness)
{
    g_animation.startColorChange(color, brightness);
}