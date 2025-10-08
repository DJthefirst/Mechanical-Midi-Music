#pragma once

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Base/SwPWM/ESP32_SwPWM.h"
    using PlatformSwPWM = ESP32_SwPWM;
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Base/SwPWM/ArduinoMega_SwPWM.h"
    using PlatformSwPWM = ArduinoMega_SwPWM;
#else
    // Default fallback - could be extended for other platforms
    // Use the regular PWM implementation as the platform-specific base
    #include "Instruments/Base/PWM/ESP32_PwmBase.h"
    using PlatformSwPWM = ESP32_PwmBase;
#endif

/**
 * Platform-selecting PWM base class
 * Automatically selects the appropriate PWM implementation based on the target platform
 * Custom instrument classes should inherit from this class to get platform-specific PWM functionality
 */
class SwPWM : public PlatformSwPWM {
public:
    /**
     * Constructor - initializes the platform-specific PWM implementation
     */
    SwPWM() : PlatformSwPWM() {}

    /**
     * Virtual destructor to ensure proper cleanup
     */
    virtual ~SwPWM() = default;

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