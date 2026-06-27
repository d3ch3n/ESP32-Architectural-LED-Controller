/**
 * @file LedController.h
 * @brief Low-level hardware abstraction layer and FastLED memory management.
 * @author Tech Lead - Ripado OS
 * @version 0.1.0-alpha
 */

#pragma once
#include "../Config/Config.h"

class LedController {
public:
    /**
     * @brief Constructor. Guarded for static initialization.
     */
    LedController() = default;
    
    /**
     * @brief Destructor. Destroys allocated pointers if necessary.
     */
    ~LedController() = default;

    /**
     * @brief Binds arrays dynamically to the physical hardware pins at runtime.
     */
    void initHardware();

    /**
     * @brief Injects a color object into a specific pixel inside an isolated channel.
     * @param stripIdx Target strip index array (0 to CONFIG_MAX_STRIPS - 1).
     * @param ledIdx Physical position of the pixel inside the mapped array.
     * @param color FastLED CRGB color structure object.
     */
    void setPixel(uint8_t stripIdx, uint16_t ledIdx, CRGB color);

    /**
     * @brief Fills all dynamic heap-allocated pixel arrays with pure black.
     */
    void clearAll();

    /**
     * @brief Drives data bits from RAM buffers directly into the physical hardware lines.
     */
    void show();

    /**
     * @brief Intercepts active master parameters and enforces current protection restrictions.
     */
    void applyBrightnessSafety();
};

// Declares the execution singleton instance for cross-module coupling
extern LedController g_ledEngine;