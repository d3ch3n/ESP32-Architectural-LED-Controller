/**
 * @file LedController.cpp
 */

#include "LedController.h"

#include <Arduino.h>

LedController g_ledEngine;

static bool attachFastLedController(uint8_t gpio,
                                    CRGB* buffer,
                                    uint16_t ledCount)
{
    switch (gpio)
    {
        case 2:
            FastLED.addLeds<WS2812B, 2, GRB>(buffer, ledCount);
            return true;

        case 4:
            FastLED.addLeds<WS2812B, 4, GRB>(buffer, ledCount);
            return true;

        case 5:
            FastLED.addLeds<WS2812B, 5, GRB>(buffer, ledCount);
            return true;

        case 12:
            FastLED.addLeds<WS2812B, 12, GRB>(buffer, ledCount);
            return true;

        case 13:
            FastLED.addLeds<WS2812B, 13, GRB>(buffer, ledCount);
            return true;

        case 14:
            FastLED.addLeds<WS2812B, 14, GRB>(buffer, ledCount);
            return true;

        case 15:
            FastLED.addLeds<WS2812B, 15, GRB>(buffer, ledCount);
            return true;

        case 16:
            FastLED.addLeds<WS2812B, 16, GRB>(buffer, ledCount);
            return true;

        case 17:
            FastLED.addLeds<WS2812B, 17, GRB>(buffer, ledCount);
            return true;

        case 18:
            FastLED.addLeds<WS2812B, 18, GRB>(buffer, ledCount);
            return true;

        case 19:
            FastLED.addLeds<WS2812B, 19, GRB>(buffer, ledCount);
            return true;

        case 21:
            FastLED.addLeds<WS2812B, 21, GRB>(buffer, ledCount);
            return true;

        case 22:
            FastLED.addLeds<WS2812B, 22, GRB>(buffer, ledCount);
            return true;

        case 23:
            FastLED.addLeds<WS2812B, 23, GRB>(buffer, ledCount);
            return true;

        case 25:
            FastLED.addLeds<WS2812B, 25, GRB>(buffer, ledCount);
            return true;

        case 26:
            FastLED.addLeds<WS2812B, 26, GRB>(buffer, ledCount);
            return true;

        case 27:
            FastLED.addLeds<WS2812B, 27, GRB>(buffer, ledCount);
            return true;

        case 32:
            FastLED.addLeds<WS2812B, 32, GRB>(buffer, ledCount);
            return true;

        case 33:
            FastLED.addLeds<WS2812B, 33, GRB>(buffer, ledCount);
            return true;

        default:
            return false;
    }
}

void LedController::initHardware()
{
    FastLED.clear(true);

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        g_strips[i].ledBuffer = nullptr;

        if (!g_strips[i].enabled || g_strips[i].ledCount == 0)
            continue;

        g_strips[i].ledBuffer = new CRGB[g_strips[i].ledCount];

        if (g_strips[i].ledBuffer == nullptr)
        {
            g_strips[i].enabled = false;
            Serial.printf("[LED] Failed to allocate strip %u\n", i + 1);
            continue;
        }

        fill_solid(g_strips[i].ledBuffer,
                   g_strips[i].ledCount,
                   CRGB::Black);

        if (!attachFastLedController(g_strips[i].gpio,
                                     g_strips[i].ledBuffer,
                                     g_strips[i].ledCount))
        {
            delete[] g_strips[i].ledBuffer;
            g_strips[i].ledBuffer = nullptr;
            g_strips[i].enabled = false;

            Serial.printf("[LED] Unsupported GPIO %u on strip %u\n",
                          g_strips[i].gpio,
                          i + 1);

            continue;
        }

        Serial.printf("[LED] Strip %u ready | GPIO %u | LEDs %u\n",
                      i + 1,
                      g_strips[i].gpio,
                      g_strips[i].ledCount);
    }

    applyBrightnessSafety();
    FastLED.clear(true);
}

void LedController::clearAll()
{
    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        clearStrip(i);
    }
}

void LedController::show()
{
    FastLED.show();
}

void LedController::applyBrightnessSafety()
{
    uint8_t safeBrightness = g_cfg.brightness;

    if (safeBrightness > g_cfg.maxBrightness)
        safeBrightness = g_cfg.maxBrightness;

    FastLED.setBrightness(safeBrightness);
}

void LedController::setPixel(uint8_t strip,
                             uint16_t led,
                             CRGB color)
{
    if (!validStrip(strip))
        return;

    if (led >= g_strips[strip].ledCount)
        return;

    g_strips[strip].ledBuffer[led] = color;
}

CRGB LedController::getPixel(uint8_t strip,
                             uint16_t led) const
{
    if (!validStrip(strip))
        return CRGB::Black;

    if (led >= g_strips[strip].ledCount)
        return CRGB::Black;

    return g_strips[strip].ledBuffer[led];
}

void LedController::clearPixel(uint8_t strip,
                               uint16_t led)
{
    setPixel(strip, led, CRGB::Black);
}

void LedController::fillStrip(uint8_t strip,
                              CRGB color)
{
    if (!validStrip(strip))
        return;

    fill_solid(g_strips[strip].ledBuffer,
               g_strips[strip].ledCount,
               color);
}

void LedController::clearStrip(uint8_t strip)
{
    fillStrip(strip, CRGB::Black);
}

uint16_t LedController::ledCount(uint8_t strip) const
{
    if (!validStrip(strip))
        return 0;

    return g_strips[strip].ledCount;
}

uint8_t LedController::stripCount() const
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < CONFIG_MAX_STRIPS; i++)
    {
        if (stripEnabled(i))
            count++;
    }

    return count;
}

bool LedController::stripEnabled(uint8_t strip) const
{
    if (strip >= CONFIG_MAX_STRIPS)
        return false;

    return g_strips[strip].enabled;
}

uint8_t LedController::gpio(uint8_t strip) const
{
    if (strip >= CONFIG_MAX_STRIPS)
        return 255;

    return g_strips[strip].gpio;
}

bool LedController::validStrip(uint8_t strip) const
{
    return strip < CONFIG_MAX_STRIPS &&
           g_strips[strip].enabled &&
           g_strips[strip].ledBuffer != nullptr;
}