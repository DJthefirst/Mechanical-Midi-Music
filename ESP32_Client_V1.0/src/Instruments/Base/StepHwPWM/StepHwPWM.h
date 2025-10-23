#pragma once

// Platform-specific includes
#if defined(ESP32)
    #include "Instruments/Base/StepHwPWM/ESP32_StepHwPWM.h"
    using PlatformStepHwPWM = ESP32_StepHwPWM;
#elif defined(ARDUINO) && !defined(ESP8266)
    #include "Instruments/Base/StepHwPWM/ArduinoMega_StepHwPWM.h"
    using PlatformStepHwPWM = ArduinoMega_StepHwPWM;
#else
    // Default fallback - could be extended for other platforms
    // Use the regular PWM implementation as the platform-specific base
    #include "Instruments/Base/PWM/ESP32_PwmBase.h"
    using PlatformStepHwPWM = ESP32_PwmBase;
#endif