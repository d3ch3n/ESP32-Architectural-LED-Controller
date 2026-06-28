/**
 * @file LedController.h
 */

#pragma once

#include <FastLED.h>
#include "../Config/Config.h"

class LedController
{
public:

    LedController() = default;
    ~LedController() = default;

    void initHardware();

    void clearAll();

    void show();

    void applyBrightnessSafety();

    void setPixel(uint8_t strip,
                  uint16_t led,
                  CRGB color);

    CRGB getPixel(uint8_t strip,
                  uint16_t led) const;

    void clearPixel(uint8_t strip,
                    uint16_t led);

    void fillStrip(uint8_t strip,
                   CRGB color);

    void clearStrip(uint8_t strip);

    uint16_t ledCount(uint8_t strip) const;

    uint8_t stripCount() const;

    bool stripEnabled(uint8_t strip) const;

    uint8_t gpio(uint8_t strip) const;

private:

    bool validStrip(uint8_t strip) const;
};

extern LedController g_ledEngine;