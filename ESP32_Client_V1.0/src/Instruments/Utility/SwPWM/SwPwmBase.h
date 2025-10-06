#pragma once

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Utility/SwPWM/ESP32_SwPwmBase.h"
    using PlatformSwPwmBase = ESP32_SwPwmBase;
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Utility/SwPWM/ArduinoMega_SwPwmBase.h"
    using PlatformSwPwmBase = ArduinoMega_SwPwmBase;
#else
    // Default fallback - could be extended for other platforms
    // Use the regular PWM implementation as the platform-specific base
    #include "Instruments/Utility/PWM/ESP32_PwmBase.h"
    using PlatformSwPwmBase = ESP32_PwmBase;
#endif

/**
 * Platform-selecting PWM base class
 * Automatically selects the appropriate PWM implementation based on the target platform
 * Custom instrument classes should inherit from this class to get platform-specific PWM functionality
 */
class SwPwmBase : public PlatformSwPwmBase {
public:
    /**
     * Constructor - initializes the platform-specific PWM implementation
     */
    SwPwmBase() : PlatformSwPwmBase() {}

    /**
     * Virtual destructor to ensure proper cleanup
     */
    virtual ~SwPwmBase() = default;

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