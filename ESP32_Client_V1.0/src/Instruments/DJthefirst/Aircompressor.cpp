// ////////////////////////////////////////////////////////////////////////////////////////////////////
// //FAST LED Helper Functions
// ////////////////////////////////////////////////////////////////////////////////////////////////////
// #ifdef EXTRA_ADDRESSABLE_LEDS

//     void PwmDriver::setupLEDs(){
//         AddrLED::get().setup();
//     }

//     //Set an Instrument Led to on
//     void PwmDriver::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
//         CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
//         AddrLED::get().turnLedOn(instrument, color);
//     }

//     //Set an Instrument Led to off
//     void PwmDriver::setInstumentLedOff(uint8_t instrument){
//         AddrLED::get().turnLedOff(instrument);
//     }

//     //Reset Leds
//     void PwmDriver::resetLEDs(){
//         AddrLED::get().reset();
//     }

// #else
// void PwmDriver::setupLEDs(){}
// void PwmDriver::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){}
// void PwmDriver::setInstumentLedOff(uint8_t instrument){}
// void PwmDriver::resetLEDs(){}
// #endif