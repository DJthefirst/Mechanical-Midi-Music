//////////////////////////////////////////////////////////////
// WARNING UNTESTED CODE - WILL NOT COMPILE! - EXAMPLE ONLY //
//////////////////////////////////////////////////////////////

#include "Instruments/Base/HwPWM/ArduinoMega_HwPWM.h"

// Only compile for Arduino platforms
#if defined(ARDUINO) && !defined(ESP32) && !defined(ESP8266) && defined(CFG_INSTRUMENT_HWPWM) && defined(CFG_COMPONENT_PWM)


#endif // Arduino platform check + CFG_INSTRUMENT_HWPWM + CFG_COMPONENT_PWM