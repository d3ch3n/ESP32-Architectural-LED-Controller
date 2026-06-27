/**
 * @file AnimationEngine.h
 * @brief Asynchronous wave processing engine driven by time-sliced states.
 * @author Tech Lead - Ripado OS
 * @version 0.1.0-alpha
 */

#pragma once
#include <Arduino.h>

/**
 * @brief Boots up timestamps and sync registers inside the core animation driver.
 */
void Animation_Init();

/**
 * @brief Updates the FSM state machine processing loops. Must be executed in main loop thread.
 */
void Animation_Update();

/**
 * @brief Triggers system transition into an upward progressive wave deployment (Floor -> Ceiling).
 */
void Animation_StartOpening();

/**
 * @brief Triggers system transition into a downward progressive wave collapse (Ceiling -> Floor).
 */
void Animation_StartClosing();

/**
 * @brief Orders a hot swap of colors with prior recoil collapse and automated upward re-deployment.
 * @param newColorHex Targeted 24-bit RGB Color configuration profile.
 * @param newBrightness Targeted luminosity output value.
 */
void Animation_StartColorChange(uint32_t newColorHex, uint8_t newBrightness);