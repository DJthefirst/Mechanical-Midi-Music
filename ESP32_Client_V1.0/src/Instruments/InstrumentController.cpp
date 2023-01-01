#include "InstrumentController.h"

void InstrumentController::reset(uint8_t instrument)
{
    //Not Yet Implemented
}

void InstrumentController::resetAll()
{
    //Not Yet Implemented
}

void InstrumentController::playNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void InstrumentController::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void InstrumentController::stopAll()
{
    //Not Yet Implemented
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t InstrumentController::getNumActiveNotes(uint8_t instrument){
    return 0;
}
bool InstrumentController::isNoteActive(uint8_t instrument, uint8_t note){
    return false;
}
void InstrumentController::setKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity){
    //Not Yet Implemented
}
void InstrumentController::setPitchBend(uint8_t instrument, uint16_t bend){
    //Not Yet Implemented
}
void InstrumentController::setProgramChange(uint8_t value){
    //Not Yet Implemented    
}
void InstrumentController::setChannelPressure(uint8_t value){
    //Not Yet Implemented   
}
void InstrumentController::setModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::setFootPedal(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::setVolume(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::setExpression(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::setEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void InstrumentController::setEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}