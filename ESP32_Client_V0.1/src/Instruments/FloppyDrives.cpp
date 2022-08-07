#include "Instruments/FloppyDrives.h"
#include "Constants.h"

void FloppyDrives::Tick()
{
    //Not Yet Implemented
}

void FloppyDrives::Reset()
{
    //Not Yet Implemented
}

void FloppyDrives::PlayNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void FloppyDrives::StopNote(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

void FloppyDrives::StopAll()
{
    //Not Yet Implemented
}

void FloppyDrives::SetKeyPressure(uint8_t instrument, uint8_t note, uint8_t velocity)
{
    //Not Yet Implemented
}

uint8_t FloppyDrives::getNumActiveNotes(uint8_t instrument)
{
    if ((_activeNotes[instrument] & MSB_BITMASK) != 0){
        return 1;
    }
    return 0;
}

bool FloppyDrives::isNoteActive(uint8_t instrument, uint8_t note)
{
    return ((_activeNotes[instrument] & (~ MSB_BITMASK)) == note &&
           ((_activeNotes[instrument] & MSB_BITMASK) != 0));

}

void FloppyDrives::SetModulationWheel(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetFootPedal(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetVolume(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetExpression(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetEffectCrtl_1(uint8_t value){
    //Not Yet Implemented
}
void FloppyDrives::SetEffectCrtl_2(uint8_t value){
    //Not Yet Implemented
}