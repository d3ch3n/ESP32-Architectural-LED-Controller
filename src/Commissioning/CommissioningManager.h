#pragma once

#include <Arduino.h>
#include <FastLED.h>

enum class CommissioningMode : uint8_t
{
    Off,
    Single,
    Fill,
    Blink,
    AutoScan
};

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

    void startBlink(uint8_t strip,
                    uint16_t led,
                    uint16_t intervalMs,
                    const CRGB& color);

    void stopBlink();

    void startAutoScan(uint8_t strip,
                       uint16_t startLed,
                       uint16_t intervalMs,
                       const CRGB& color);

    void stopAutoScan();

    bool isRunning() const;
    bool isBlinking() const;
    bool isAutoScanRunning() const;
    CommissioningMode currentMode() const;
    uint8_t currentStrip() const;
    uint16_t currentLed() const;

private:
    struct Session
    {
        CommissioningMode mode = CommissioningMode::Off;
        uint8_t strip = 0;
        uint16_t led = 0;
        uint16_t count = 0;
        uint16_t intervalMs = 120;
        CRGB color = CRGB::White;
        bool running = false;
        bool blinkState = false;
        unsigned long lastTick = 0;
    };

    void normalizeStrip();
    void showCurrentAutoScanLed();
    void renderBlink();

    Session m_session;
};

extern CommissioningManager g_commissioning;
