#pragma once

#if defined(CFG_INSTRUMENT_HWPWM)
    #include "Instruments/Base/HwPWM/HwPWM.h"
#elif defined(CFG_INSTRUMENT_SWPWM)
    #include "Instruments/Base/SwPWM/SwPWM.h"
#else
    #include "Instruments/Base/SwPWM/SwPWM.h"
#endif
