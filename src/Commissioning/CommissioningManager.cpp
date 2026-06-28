#include "CommissioningManager.h"

#include "../Config/Config.h"
#include "../Led/LedController.h"
#include "../Storage/StorageManager.h"

CommissioningManager g_commissioning;

void CommissioningManager::begin()
{
    m_session.mode = CommissioningMode::Off;
    m_session.strip = 0;
    m_session.led = 0;
    m_session.count = 0;
    m_session.intervalMs = 120;
    m_session.color = CRGB::White;
    m_session.running = false;
    m_session.blinkState = true;
    m_session.lastTick = 0;
}

void CommissioningManager::update()
{
    unsigned long now = millis();

    if (m_session.mode == CommissioningMode::Blink)
    {
        if (now - m_session.lastTick >= m_session.intervalMs)
        {
            m_session.lastTick = now;
            m_session.blinkState = !m_session.blinkState;
        }

        renderBlink();
        return;
    }

    if (m_session.mode != CommissioningMode::AutoScan)
        return;

    if (now - m_session.lastTick < m_session.intervalMs)
        return;

    m_session.lastTick = now;

    uint16_t count = g_ledEngine.ledCount(m_session.strip);

    if (count == 0)
    {
        stopAutoScan();
        return;
    }

    showCurrentAutoScanLed();

    if (m_session.led < count - 1)
    {
        m_session.led++;
    }
    else
    {
        m_session.mode = CommissioningMode::Off;
        m_session.running = false;

        Serial.printf(
            "[COMMISSIONING] AUTO SCAN FINISHED | Strip: %u | Last LED: %u\n",
            m_session.strip + 1,
            m_session.led);
    }
}

void CommissioningManager::off()
{
    m_session.mode = CommissioningMode::Off;
    m_session.running = false;
    m_session.blinkState = false;

    Serial.println("[COMMISSIONING] OFF");

    g_ledEngine.clearAll();
    g_ledEngine.show();
}

void CommissioningManager::showSingle(uint8_t strip,
                                      uint16_t led,
                                      const CRGB& color)
{
    m_session.mode = CommissioningMode::Single;
    m_session.running = false;
    m_session.strip = strip;
    m_session.led = led;
    m_session.color = color;
    m_session.blinkState = true;
    m_session.lastTick = 0;

    normalizeStrip();

    Serial.printf(
        "[COMMISSIONING] SINGLE | Strip: %u | LED: %u | RGB(%u,%u,%u)\n",
        m_session.strip + 1,
        m_session.led,
        m_session.color.r,
        m_session.color.g,
        m_session.color.b);

    g_ledEngine.clearAll();
    g_ledEngine.setPixel(m_session.strip, m_session.led, m_session.color);
    g_ledEngine.show();
}

void CommissioningManager::fillTo(uint8_t strip,
                                  uint16_t led,
                                  const CRGB& color)
{
    m_session.mode = CommissioningMode::Fill;
    m_session.running = false;
    m_session.strip = strip;
    m_session.led = led;
    m_session.color = color;
    m_session.blinkState = false;
    m_session.lastTick = 0;

    normalizeStrip();

    Serial.printf(
        "[COMMISSIONING] FILL | Strip: %u | LED: %u | RGB(%u,%u,%u)\n",
        m_session.strip + 1,
        m_session.led,
        m_session.color.r,
        m_session.color.g,
        m_session.color.b);

    g_ledEngine.clearAll();

    uint16_t count = g_ledEngine.ledCount(m_session.strip);

    if (count == 0)
    {
        Serial.println("[COMMISSIONING] Invalid strip or zero LEDs.");
        g_ledEngine.show();
        return;
    }

    if (m_session.led >= count)
        m_session.led = count - 1;

    for (uint16_t i = 0; i <= m_session.led; i++)
    {
        g_ledEngine.setPixel(m_session.strip, i, m_session.color);
    }

    g_ledEngine.show();
}

void CommissioningManager::saveCount(uint8_t strip,
                                     uint16_t count)
{
    m_session.mode = CommissioningMode::Off;
    m_session.running = false;
    m_session.strip = strip;
    m_session.count = count;

    normalizeStrip();

    if (count == 0)
        return;

    g_strips[m_session.strip].ledCount = count;
    g_strips[m_session.strip].enabled = true;

    g_storage.saveConfiguration();

    Serial.printf(
        "[COMMISSIONING] SAVE COUNT | Strip: %u | LEDs: %u\n",
        m_session.strip + 1,
        count);
}

void CommissioningManager::startBlink(uint8_t strip,
                                      uint16_t led,
                                      uint16_t intervalMs,
                                      const CRGB& color)
{
    m_session.mode = CommissioningMode::Blink;
    m_session.running = true;
    m_session.strip = strip;
    m_session.led = led;
    m_session.intervalMs = intervalMs;
    m_session.color = color;
    m_session.blinkState = true;
    m_session.lastTick = millis();

    normalizeStrip();

    if (m_session.intervalMs < 80)
        m_session.intervalMs = 80;

    uint16_t count = g_ledEngine.ledCount(m_session.strip);

    if (count == 0)
    {
        Serial.println("[COMMISSIONING] BLINK FAILED | Invalid strip or zero LEDs.");
        off();
        return;
    }

    if (m_session.led >= count)
        m_session.led = count - 1;

    Serial.printf(
        "[COMMISSIONING] BLINK START | Strip: %u | LED: %u | Interval: %u ms | RGB(%u,%u,%u)\n",
        m_session.strip + 1,
        m_session.led,
        m_session.intervalMs,
        m_session.color.r,
        m_session.color.g,
        m_session.color.b);

    renderBlink();
}

void CommissioningManager::stopBlink()
{
    if (m_session.mode == CommissioningMode::Blink)
    {
        Serial.printf(
            "[COMMISSIONING] BLINK STOP | Strip: %u | LED: %u\n",
            m_session.strip + 1,
            m_session.led);
    }

    off();
}

void CommissioningManager::startAutoScan(uint8_t strip,
                                         uint16_t startLed,
                                         uint16_t intervalMs,
                                         const CRGB& color)
{
    m_session.strip = strip;
    normalizeStrip();

    uint16_t count = g_ledEngine.ledCount(m_session.strip);

    if (count == 0)
    {
        Serial.println("[COMMISSIONING] AUTO SCAN FAILED | Invalid strip or zero LEDs.");
        return;
    }

    if (startLed >= count)
        startLed = count - 1;

    if (intervalMs < 30)
        intervalMs = 30;

    m_session.mode = CommissioningMode::AutoScan;
    m_session.running = true;
    m_session.led = startLed;
    m_session.intervalMs = intervalMs;
    m_session.color = color;
    m_session.blinkState = true;
    m_session.lastTick = 0;

    Serial.printf(
        "[COMMISSIONING] AUTO SCAN START | Strip: %u | Start LED: %u | Interval: %u ms\n",
        m_session.strip + 1,
        m_session.led,
        m_session.intervalMs);

    showCurrentAutoScanLed();
}

void CommissioningManager::stopAutoScan()
{
    if (m_session.mode == CommissioningMode::AutoScan)
    {
        Serial.printf(
            "[COMMISSIONING] AUTO SCAN STOP | Strip: %u | LED: %u\n",
            m_session.strip + 1,
            m_session.led);
    }

    m_session.mode = CommissioningMode::Off;
    m_session.running = false;
}

bool CommissioningManager::isRunning() const
{
    return m_session.running;
}

bool CommissioningManager::isBlinking() const
{
    return m_session.mode == CommissioningMode::Blink;
}

bool CommissioningManager::isAutoScanRunning() const
{
    return m_session.mode == CommissioningMode::AutoScan;
}

CommissioningMode CommissioningManager::currentMode() const
{
    return m_session.mode;
}

uint8_t CommissioningManager::currentStrip() const
{
    return m_session.strip;
}

uint16_t CommissioningManager::currentLed() const
{
    return m_session.led;
}

void CommissioningManager::normalizeStrip()
{
    if (m_session.strip >= CONFIG_MAX_STRIPS)
        m_session.strip = 0;
}

void CommissioningManager::showCurrentAutoScanLed()
{
    g_ledEngine.clearAll();
    g_ledEngine.setPixel(m_session.strip, m_session.led, m_session.color);
    g_ledEngine.show();

    Serial.printf(
        "[COMMISSIONING] AUTO SCAN LED | Strip: %u | LED: %u\n",
        m_session.strip + 1,
        m_session.led);
}

void CommissioningManager::renderBlink()
{
    g_ledEngine.clearAll();

    if (m_session.blinkState)
    {
        g_ledEngine.setPixel(m_session.strip, m_session.led, m_session.color);
    }

    g_ledEngine.show();
}
