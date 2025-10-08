#pragma once

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Base/HwPWM/ESP32_HwPWM.h"
    using PlatformHwPWM = ESP32_HwPWM;
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Base/HwPWM/ArduinoMega_HwPWM.h"
    using PlatformHwPWM = ArduinoMega_HwPWM;
#else
    // Default fallback - could be extended for other platforms
    #include "Instruments/Base/HwPWM/ESP32_HwPWM.h"
    using PlatformHwPWM = ESP32_HwPWM;
#endif

/**
 * Platform-selecting PWM base class
 * Automatically selects the appropriate PWM implementation based on the target platform
 * Custom instrument classes should inherit from this class to get platform-specific PWM functionality
 */
class HwPWM : public PlatformHwPWM {
public:
    /**
     * Constructor - initializes the platform-specific PWM implementation
     */
    HwPWM() : PlatformHwPWM() {}

    /**
     * Virtual destructor to ensure proper cleanup
     */
    virtual ~HwPWM() = default;

    // All PWM functionality is inherited from the platform-specific implementation
    // Additional common functionality can be added here if needed
    
    /**
     * Get the platform name for debugging purposes
     * @return Platform name string
     */
    static const char* getPlatformName() {
#if defined(ESP32)
        return "ESP32";
#elif defined(ARDUINO) && !defined(ESP8266)
        return "Arduino Mega";
#else
        return "Unknown/Default";
#endif
    }
};