// #include "InstrumentController.h"

// #include "Instruments/InstrumentController.h"
// #include "Instruments/PwmDriver.h"
// #include "Instruments/FloppyDrive.h"
// #include "Instruments/DrumSimple.h"
// #include "Instruments/ShiftRegister.h"
// #include "Instruments/DJthefirst/Dulcimer.h"

// enum class InstrumentType {
//     DrumSimple,
//     FloppyDrive,
//     PwmDriver,
//     ShiftRegister,
//     Dulcimer
// };

// class Instrument {
    
//     private:
//         static InstrumentController* m_instrumentController;

//     public:

//         template<InstrumentType INSTRUMENT_TYPE>
//         Instrument(uint8_t* pins, uint8_t numInstruments){
//             switch(INSTRUMENT_TYPE) {
//                 case InstrumentType::DrumSimple:    { m_instrumentController = DrumSimple(uint8_t* pins, uint8_t numInstruments); }
//                 case InstrumentType::FloppyDrive:   { m_instrumentController = FloppyDrive(uint8_t* pins, uint8_t numInstruments); }
//                 case InstrumentType::PwmDriver:     { m_instrumentController = PwmDriver(uint8_t* pins, uint8_t numInstruments); }
//                 case InstrumentType::ShiftRegister: { m_instrumentController = ShiftRegister(uint8_t* pins, uint8_t numInstruments); }
//                 case InstrumentType::Dulcimer:      { m_instrumentController = Dulcimer(uint8_t* pins, uint8_t numInstruments); }
//             }
//         }
// };