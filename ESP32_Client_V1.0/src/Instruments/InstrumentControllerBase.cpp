#include "InstrumentControllerBase.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

// uint8_t InstrumentController::getNumActiveNotes(uint8_t instrument){
//     return 0;
// }
// bool InstrumentController::isNoteActive(uint8_t instrument, uint8_t note){
//     return false;
// }
void InstrumentControllerBase::setKeyPressure(uint8_t channel, uint8_t note, uint8_t velocity){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setPitchBend(uint8_t channel,uint16_t bend){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setProgramChange(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setChannelPressure(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setModulationWheel(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setFootPedal(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setVolume(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setExpression(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setEffectCrtl_1(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}
void InstrumentControllerBase::setEffectCrtl_2(uint8_t channel, uint8_t value){
    // Default implementation does nothing - derived classes should override if needed
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Distributor Tracking Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void InstrumentControllerBase::checkInstrumentTimeouts(){
    // Default implementation does nothing - derived classes should override if needed
}
