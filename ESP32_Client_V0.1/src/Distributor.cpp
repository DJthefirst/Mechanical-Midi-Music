#include "Distributor.h"

Distributor::Distributor(InstrumentController* ptrInstrumentController)
{
    ptr_InstrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Message Processor
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::processMessage(uint8_t message[])
{
    currentChannel = (message[0] & 0b00001111);

    //Determine Type of Msg and Call Associated Event
    uint8_t msg_type = (message[0] & 0b11110000);
    
    switch(msg_type){

    case(NoteOff):
        NoteOffEvent(message[1],message[2]);
        break; 
    case(NoteOn):
        NoteOnEvent(message[1],message[2]);
        break;
    case(KeyPressure):
        KeyPressureEvent(message[1],message[2]);
        break;
    case(ControlChange):
        ControlChangeEvent(message[1],message[2]);
        break;
    case(ProgramChange):
        ProgramChangeEvent(message[1]);
        break;
    case(ChannelPressure):
        ChannelPressureEvent(message[1]);
        break;
    case(PitchBend):
        PitchBendEvent((message[2] << 8) & message[1]);
        break;
    case(SysCommon):
        break;
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Event Handlers
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::NoteOffEvent(uint8_t Note, uint8_t Velocity)
{
    uint8_t instrument = CheckForNote(Note);
    if(instrument != 0xFF){
        (*ptr_InstrumentController).StopNote(instrument, Note, Velocity);
    }
}

void Distributor::NoteOnEvent(uint8_t Note, uint8_t Velocity)
{
    uint8_t instrument = CheckForNote(Note);

    if((Velocity == 0) && (instrument != 0xFF)){
        (*ptr_InstrumentController).StopNote(instrument, Note, Velocity);
    }
    else if(instrument != 0xFF){
        (*ptr_InstrumentController).PlayNote(instrument, Note, Velocity);
    }
}

void Distributor::KeyPressureEvent(uint8_t Note, uint8_t Velocity)
{
    uint8_t instrument = CheckForNote(Note);
    if(instrument != 0xFF){
        (*ptr_InstrumentController).SetKeyPressure(instrument, Note, Velocity);
    }
}

void Distributor::ControlChangeEvent(uint8_t Controller, uint8_t Value)
{
    //Not Yet Implemented
}

void Distributor::ProgramChangeEvent(uint8_t Program)
{
    //Not Yet Implemented
}

void Distributor::ChannelPressureEvent(uint8_t Velocity)
{
    //Not Yet Implemented
}

void Distributor::PitchBendEvent(uint16_t PitchBend)
{
    //Not Yet Implemented
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Distributor::NextInstrument()
{
    uint8_t insturmentLeastActive = 0;
    uint8_t leastActiveNotes = 255;

    switch(_distributionMethod){

    case(StrightThrough):
        currentInstrument = currentChannel;
        return currentInstrument;


    case(RoundRobin):

        for(int i = 0; i < 32; i++){
            currentInstrument++;

            //Loop to first instrument if end is reached
            currentInstrument = (currentInstrument >= 32) ? 0 : currentInstrument;

            //Check if valid instrument
            if(_instruments & (1 << currentInstrument) != 0){
                return currentInstrument;
            }
        }
        return currentInstrument;


    case(Accending):

        for(int i = 0; (i < 32); i++){
            if(_instruments & (1 << i) != 0)
            {
                uint8_t activeNotes = (*ptr_InstrumentController).getNumActiveNotes(i);

                if(activeNotes == 0)
                {
                    return insturmentLeastActive;
                }

                if(activeNotes < leastActiveNotes)
                {
                    leastActiveNotes = activeNotes;
                    insturmentLeastActive = i;
                }
            }
        }
        return insturmentLeastActive;

    case(Decending):

        for(int i = 31; (i >= 0); i--){
            if(_instruments & (1 << i) != 0)
            {
                uint8_t activeNotes = (*ptr_InstrumentController).getNumActiveNotes(i);

                if(activeNotes == 0)
                {
                    return insturmentLeastActive;
                }

                if(activeNotes < leastActiveNotes)
                {
                    leastActiveNotes = activeNotes;
                    insturmentLeastActive = i;
                }
            }
        }
        return insturmentLeastActive;
    }
}

uint8_t Distributor::CheckForNote(uint8_t note)
{
    uint8_t nextInstrument = currentInstrument;

    switch(_distributionMethod){

    case(StrightThrough):
        if((*ptr_InstrumentController).isNoteActive(currentChannel, note)){
            return currentChannel;
        }
        return 0xFF;

    case(RoundRobin):
        for(int i = 0; i < 32; i++){
            nextInstrument--;

            nextInstrument = (nextInstrument < 0) ? 31 : nextInstrument;

            //Check if valid instrument
            if(_instruments & (1 << nextInstrument) != 0){
                if((*ptr_InstrumentController).isNoteActive(nextInstrument, note)){
                    return nextInstrument;
                }
            }
        }
        return 0xFF;

    case(Accending):
        for(int i = 0; i < 32; i++){

            //Check if valid instrument
            if(_instruments & (1 << i) != 0){
                if((*ptr_InstrumentController).isNoteActive(i, note)){
                    return i;
                }
            }
        }
        return 0xFF;

    case(Decending):
        for(int i = 31; i >= 0; i--){

            //Check if valid instrument
            if(_instruments & (1 << i) != 0){
                if((*ptr_InstrumentController).isNoteActive(i, note)){
                    return i;
                }
            }
        }
        return 0xFF;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters
////////////////////////////////////////////////////////////////////////////////////////////////////

//output Array = uint8_t[9]
void Distributor::getDistributor(uint8_t* outputArray)
{
    uint8_t dstributorObj[] = {((_noteOverwrite << 7) & (_numPolyphonicNotes)), _minNote, _maxNote, _channels >> 8, _channels, 
        _instruments >> 24, _instruments >> 16, _instruments >> 8, _instruments};
    outputArray = dstributorObj;
}

uint16_t Distributor::getChannels(){
    return _channels;
}

void Distributor::setNoteOverwrite(bool noteOverwrite)
{
    _noteOverwrite = noteOverwrite;
}

void Distributor::setMinMaxNote(uint8_t minNote, uint8_t maxNote)
{
    _minNote = minNote;
    _maxNote = maxNote;
}

void Distributor::setNumPolyphonicNotes(uint8_t numPolyphonicNotes)
{
    _numPolyphonicNotes = numPolyphonicNotes;
}
    
void Distributor::setChannels(uint16_t channels)
{
    _channels = channels;
}
    
void Distributor::setInstruments(uint32_t instruments)
{
    _instruments = instruments;
}