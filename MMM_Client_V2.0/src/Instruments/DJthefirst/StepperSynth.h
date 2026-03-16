#pragma once

#if defined(CFG_INSTRUMENT_STEPPERSYNTHHW)
    #include "Instruments/DJthefirst/StepperSynthHw.h"
#elif defined(CFG_INSTRUMENT_STEPPERSYNTHSW)
    #include "Instruments/DJthefirst/StepperSynthSw.h"
#else
    #include "Instruments/DJthefirst/StepperSynthHw.h"
#endif
