#pragma once

#include <Arduino.h>
#include <FastLED.h>

class CommissioningManager
{
public:
    CommissioningManager() = default;

    void begin();
    void update();

    void off();

    void showSingle(uint8_t strip,
                    uint16_t led,
                    const CRGB& color);

    void fillTo(uint8_t strip,
                uint16_t led,
                const CRGB& color);

    void saveCount(uint8_t strip,
                   uint16_t count);

    void startAutoScan(uint8_t strip,
                       uint16_t startLed,
                       uint16_t intervalMs,
                       const CRGB& color);

    void stopAutoScan();

    bool isAutoScanRunning() const;
    uint8_t currentStrip() const;
    uint16_t currentLed() const;

private:
    void showCurrentAutoScanLed();

    bool m_autoScanRunning = false;

    uint8_t m_currentStrip = 0;
    uint16_t m_currentLed = 0;
    uint16_t m_autoScanIntervalMs = 120;

    CRGB m_autoScanColor = CRGB::White;

    unsigned long m_lastAutoScanStep = 0;
};

extern CommissioningManager g_commissioning;