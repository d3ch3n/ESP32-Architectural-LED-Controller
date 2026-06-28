#pragma once

#include <Arduino.h>
#include <FastLED.h>

class CommandManager
{
public:
    CommandManager() = default;

    void begin();
    void update();

    void powerOn();
    void powerOff();
    void togglePower();

    void setBrightness(uint8_t brightness);
    void setColor(const CRGB& color);
    void setColor(uint32_t colorHex);

    bool isOn() const;
    uint8_t brightness() const;
    CRGB color() const;

private:
    bool m_power = false;
    uint8_t m_brightness = 255;
    CRGB m_color = CRGB::White;
};

extern CommandManager g_commandManager;