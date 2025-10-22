#pragma once

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Base/StepSwPWM/ESP32_StepSwPWM.h"
    using PlatformStepSwPWM = ESP32_StepSwPWM;
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Base/StepSwPWM/ArduinoMega_StepSwPWM.h"
    using PlatformStepSwPWM = ArduinoMega_StepSwPWM;
#else
    // Default fallback - could be extended for other platforms
    // Use the regular PWM implementation as the platform-specific base
    #include "Instruments/Base/PWM/ESP32_PwmBase.h"
    using PlatformStepSwPWM = ESP32_PwmBase;
#endif