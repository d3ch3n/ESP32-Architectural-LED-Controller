#include "CommissioningManager.h"

#include "../Config/Config.h"
#include "../Led/LedController.h"
#include "../Storage/StorageManager.h"

CommissioningManager g_commissioning;

void CommissioningManager::begin()
{
    m_autoScanRunning = false;
    m_currentStrip = 0;
    m_currentLed = 0;
    m_autoScanIntervalMs = 120;
    m_autoScanColor = CRGB::White;
    m_lastAutoScanStep = 0;
}

void CommissioningManager::update()
{
    if (!m_autoScanRunning)
        return;

    unsigned long now = millis();

    if (now - m_lastAutoScanStep < m_autoScanIntervalMs)
        return;

    m_lastAutoScanStep = now;

    uint16_t count = g_ledEngine.ledCount(m_currentStrip);

    if (count == 0)
    {
        stopAutoScan();
        return;
    }

    showCurrentAutoScanLed();

    if (m_currentLed < count - 1)
    {
        m_currentLed++;
    }
    else
    {
        m_autoScanRunning = false;

        Serial.printf(
            "[COMMISSIONING] AUTO SCAN FINISHED | Strip: %u | Last LED: %u\n",
            m_currentStrip + 1,
            m_currentLed);
    }
}

void CommissioningManager::off()
{
    m_autoScanRunning = false;

    Serial.println("[COMMISSIONING] OFF");

    g_ledEngine.clearAll();
    g_ledEngine.show();
}

void CommissioningManager::showSingle(uint8_t strip,
                                      uint16_t led,
                                      const CRGB& color)
{
    m_autoScanRunning = false;

    if (strip >= CONFIG_MAX_STRIPS)
        strip = 0;

    Serial.printf(
        "[COMMISSIONING] SINGLE | Strip: %u | LED: %u | RGB(%u,%u,%u)\n",
        strip + 1,
        led,
        color.r,
        color.g,
        color.b);

    g_ledEngine.clearAll();
    g_ledEngine.setPixel(strip, led, color);
    g_ledEngine.show();

    m_currentStrip = strip;
    m_currentLed = led;
}

void CommissioningManager::fillTo(uint8_t strip,
                                  uint16_t led,
                                  const CRGB& color)
{
    m_autoScanRunning = false;

    if (strip >= CONFIG_MAX_STRIPS)
        strip = 0;

    Serial.printf(
        "[COMMISSIONING] FILL | Strip: %u | LED: %u | RGB(%u,%u,%u)\n",
        strip + 1,
        led,
        color.r,
        color.g,
        color.b);

    g_ledEngine.clearAll();

    uint16_t count = g_ledEngine.ledCount(strip);

    if (count == 0)
    {
        Serial.println("[COMMISSIONING] Invalid strip or zero LEDs.");
        g_ledEngine.show();
        return;
    }

    if (led >= count)
        led = count - 1;

    for (uint16_t i = 0; i <= led; i++)
    {
        g_ledEngine.setPixel(strip, i, color);
    }

    g_ledEngine.show();

    m_currentStrip = strip;
    m_currentLed = led;
}

void CommissioningManager::saveCount(uint8_t strip,
                                     uint16_t count)
{
    m_autoScanRunning = false;

    if (strip >= CONFIG_MAX_STRIPS)
        strip = 0;

    if (count == 0)
        return;

    g_strips[strip].ledCount = count;
    g_strips[strip].enabled = true;

    g_storage.saveConfiguration();

    Serial.printf(
        "[COMMISSIONING] SAVE COUNT | Strip: %u | LEDs: %u\n",
        strip + 1,
        count);
}

void CommissioningManager::startAutoScan(uint8_t strip,
                                         uint16_t startLed,
                                         uint16_t intervalMs,
                                         const CRGB& color)
{
    if (strip >= CONFIG_MAX_STRIPS)
        strip = 0;

    uint16_t count = g_ledEngine.ledCount(strip);

    if (count == 0)
    {
        Serial.println("[COMMISSIONING] AUTO SCAN FAILED | Invalid strip or zero LEDs.");
        return;
    }

    if (startLed >= count)
        startLed = count - 1;

    if (intervalMs < 30)
        intervalMs = 30;

    m_currentStrip = strip;
    m_currentLed = startLed;
    m_autoScanIntervalMs = intervalMs;
    m_autoScanColor = color;
    m_lastAutoScanStep = 0;
    m_autoScanRunning = true;

    Serial.printf(
        "[COMMISSIONING] AUTO SCAN START | Strip: %u | Start LED: %u | Interval: %u ms\n",
        strip + 1,
        startLed,
        intervalMs);

    showCurrentAutoScanLed();
}

void CommissioningManager::stopAutoScan()
{
    if (m_autoScanRunning)
    {
        Serial.printf(
            "[COMMISSIONING] AUTO SCAN STOP | Strip: %u | LED: %u\n",
            m_currentStrip + 1,
            m_currentLed);
    }

    m_autoScanRunning = false;
}

bool CommissioningManager::isAutoScanRunning() const
{
    return m_autoScanRunning;
}

uint8_t CommissioningManager::currentStrip() const
{
    return m_currentStrip;
}

uint16_t CommissioningManager::currentLed() const
{
    return m_currentLed;
}

void CommissioningManager::showCurrentAutoScanLed()
{
    g_ledEngine.clearAll();
    g_ledEngine.setPixel(m_currentStrip, m_currentLed, m_autoScanColor);
    g_ledEngine.show();

    Serial.printf(
        "[COMMISSIONING] AUTO SCAN LED | Strip: %u | LED: %u\n",
        m_currentStrip + 1,
        m_currentLed);
}