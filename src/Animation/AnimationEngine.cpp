#include "AnimationEngine.h"

AnimationEngine g_animation;

void AnimationEngine::begin()
{
    calculateOffsets();
    currentState = STATE_OFF;
}

void AnimationEngine::calculateOffsets()
{
    maxSystemLeds = 0;

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        if (g_strips[i].enabled &&
            g_strips[i].ledCount > maxSystemLeds)
        {
            maxSystemLeds = g_strips[i].ledCount;
        }
    }

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        if (g_strips[i].enabled)
        {
            g_strips[i].offset =
                maxSystemLeds - g_strips[i].ledCount;
        }
    }
}

void AnimationEngine::startOpening()
{
    colorTransitionActive = false;

    animationStep = 0;

    g_cfg.power = true;

    currentState = STATE_POWERING_ON;
}

void AnimationEngine::startClosing()
{
    colorTransitionActive = false;

    animationStep = maxSystemLeds - 1;

    g_cfg.power = false;

    currentState = STATE_POWERING_OFF;
}

void AnimationEngine::startColorChange(uint32_t targetColorHex,
                                       uint8_t targetBrightness)
{
    pendingColor = targetColorHex;

    g_cfg.brightness = targetBrightness;

    g_ledEngine.applyBrightnessSafety();

    colorTransitionActive = true;

    animationStep = maxSystemLeds - 1;

    currentState = STATE_POWERING_OFF;
}

void AnimationEngine::update()
{
    if (currentState == STATE_OFF ||
        currentState == STATE_ON)
        return;

    unsigned long now = millis();

    if (now - lastStepTimestamp < g_cfg.animationSpeed)
        return;

    lastStepTimestamp = now;

    switch (currentState)
    {
        case STATE_POWERING_ON:

            if (animationStep < maxSystemLeds)
            {
                for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
                {
                    if (!g_strips[i].enabled)
                        continue;

                    int16_t led =
                        animationStep - g_strips[i].offset;

                    if (led >= 0 &&
                        led < g_strips[i].ledCount)
                    {
                        g_ledEngine.setPixel(
                            i,
                            led,
                            CRGB(g_cfg.colorHex));
                    }
                }

                animationStep++;
            }
            else
            {
                currentState = STATE_ON;
                colorTransitionActive = false;
            }

            break;

        case STATE_POWERING_OFF:

            if (animationStep >= 0)
            {
                for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
                {
                    if (!g_strips[i].enabled)
                        continue;

                    int16_t led =
                        g_strips[i].ledCount - 1 -
                        (maxSystemLeds - 1 - animationStep);

                    if (led >= 0 &&
                        led < g_strips[i].ledCount)
                    {
                        g_ledEngine.setPixel(
                            i,
                            led,
                            CRGB::Black);
                    }
                }

                animationStep--;
            }
            else
            {
                if (colorTransitionActive)
                {
                    g_cfg.colorHex = pendingColor;

                    animationStep = 0;

                    currentState = STATE_POWERING_ON;
                }
                else
                {
                    currentState = STATE_OFF;
                }
            }

            break;

        default:
            break;
    }
}

/*
|--------------------------------------------------------------------------
| Compatibilidade com o código atual
|--------------------------------------------------------------------------
*/

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
    g_animation.startColorChange(color,
                                 brightness);
}