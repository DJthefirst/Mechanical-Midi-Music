#include "InstrumentController.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

// uint8_t InstrumentController::getNumActiveNotes(uint8_t instrument){
//     return 0;
// }
// bool InstrumentController::isNoteActive(uint8_t instrument, uint8_t note){
//     return false;
// }
void InstrumentController::setKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity){
    // Not yet implemented
}
void InstrumentController::setPitchBend(uint8_t instrument, uint16_t bend, uint8_t channel){
    // Not yet implemented
}
void InstrumentController::setProgramChange(uint8_t value){
    // Not yet implemented    
}
void InstrumentController::setChannelPressure(uint8_t value){
    // Not yet implemented   
}
void InstrumentController::setModulationWheel(uint8_t value){
    // Not yet implemented
}
void InstrumentController::setFootPedal(uint8_t value){
    // Not yet implemented
}
void InstrumentController::setVolume(uint8_t value){
    // Not yet implemented
}
void InstrumentController::setExpression(uint8_t value){
    // Not yet implemented
}
void InstrumentController::setEffectCrtl_1(uint8_t value){
    // Not yet implemented
}
void InstrumentController::setEffectCrtl_2(uint8_t value){
    // Not yet implemented
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Distributor Tracking Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void InstrumentController::setLastDistributor(uint8_t instrument, void* distributor){
    // Default implementation does nothing - derived classes should override if needed
}

void* InstrumentController::getLastDistributor(uint8_t instrument){
    // Default implementation returns nullptr - derived classes should override if needed
    return nullptr;
}

void InstrumentController::clearDistributorFromInstrument(void* distributor){
    // Default implementation does nothing - derived classes should override if needed
}

void InstrumentController::checkInstrumentTimeouts(){
    // Default implementation does nothing - derived classes should override if needed
}