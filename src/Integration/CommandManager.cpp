#include "CommandManager.h"

#include "../Animation/AnimationEngine.h"
#include "../Config/Config.h"
#include "../Led/LedController.h"

CommandManager g_commandManager;

void CommandManager::begin()
{
    m_power = g_cfg.power;
    m_brightness = g_cfg.brightness;
    m_color = CRGB(g_cfg.colorHex);
}

void CommandManager::update()
{
}

void CommandManager::powerOn()
{
    if (m_power)
        return;

    m_power = true;
    g_cfg.power = true;

    Animation_StartOpening();
}

void CommandManager::powerOff()
{
    if (!m_power)
        return;

    m_power = false;
    g_cfg.power = false;

    Animation_StartClosing();
}

void CommandManager::togglePower()
{
    if (m_power)
        powerOff();
    else
        powerOn();
}

void CommandManager::setBrightness(uint8_t brightness)
{
    m_brightness = brightness;
    g_cfg.brightness = brightness;

    g_ledEngine.applyBrightnessSafety();
}

void CommandManager::setColor(const CRGB& color)
{
    uint32_t colorHex =
        ((uint32_t)color.r << 16) |
        ((uint32_t)color.g << 8) |
        color.b;

    setColor(colorHex);
}

void CommandManager::setColor(uint32_t colorHex)
{
    m_color = CRGB(colorHex);

    if (m_power)
    {
        Animation_StartColorChange(colorHex, m_brightness);
    }
    else
    {
        g_cfg.colorHex = colorHex;
    }
}

bool CommandManager::isOn() const
{
    return m_power;
}

uint8_t CommandManager::brightness() const
{
    return m_brightness;
}

CRGB CommandManager::color() const
{
    return m_color;
}