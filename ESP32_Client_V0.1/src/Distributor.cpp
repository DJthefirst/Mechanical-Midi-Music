/*
Distributors serve to route midi notes to varrious instrument groups. ex. Midi ch 2 & 3 -> instruments 4-8
*/
#include "Distributor.h"
#include "Arduino.h"

Distributor::Distributor(InstrumentController* ptrInstrumentController)
{
    _ptrInstrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Message Processor
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::processMessage(uint8_t message[])
{
    _currentChannel = (message[0] & 0b00001111);

    //Determine Type of MIDI Msg and Call Associated Event
    uint8_t msg_type = (message[0] & 0b11110000);
    
    switch(msg_type){

    case(MIDI_NoteOff):
        NoteOffEvent(message[1],message[2]);
        break; 
    case(MIDI_NoteOn):
        NoteOnEvent(message[1],message[2]);
        break;
    case(MIDI_KeyPressure):
        KeyPressureEvent(message[1],message[2]);
        break;
    case(MIDI_ControlChange):
        ControlChangeEvent(message[1],message[2]);
        break;
    case(MIDI_ProgramChange):
        ProgramChangeEvent(message[1]);
        break;
    case(MIDI_ChannelPressure):
        ChannelPressureEvent(message[1]);
        break;
    case(MIDI_PitchBend):
        PitchBendEvent((message[2] << 7) | message[1]);
        break;
    case(MIDI_SysCommon):
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Event Handlers
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::NoteOffEvent(uint8_t Note, uint8_t Velocity)
{
    uint8_t instrument = CheckForNote(Note); //returns -1 if no instrument found
    if(instrument != NONE){ 
        (*_ptrInstrumentController).StopNote(instrument, Note, Velocity);
    }
}

void Distributor::NoteOnEvent(uint8_t Note, uint8_t Velocity)
{
    //Check if note has 0 velocity for note off
    if((Velocity == 0)){
        NoteOffEvent(Note, Velocity);
    }
    else if(Velocity != 0){
        uint8_t instrument = NextInstrument();
        if(instrument != NONE){ 
            (*_ptrInstrumentController).PlayNote(instrument, Note, Velocity);
        }
    }
}

void Distributor::KeyPressureEvent(uint8_t Note, uint8_t Velocity)
{
    uint8_t instrument = CheckForNote(Note);
    if(instrument != NONE){
        (*_ptrInstrumentController).SetKeyPressure(instrument, Note, Velocity);
    }
}

void Distributor::ControlChangeEvent(uint8_t Controller, uint8_t Value)
{
    //Implemented in MessageHandler
}

void Distributor::ProgramChangeEvent(uint8_t Program)
{
    //Not Yet Implemented
}

void Distributor::ChannelPressureEvent(uint8_t Velocity)
{
    //Not Yet Implemented
}

void Distributor::PitchBendEvent(uint16_t pitchBend)
{
    for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
        if((_instruments & (1 << i)) != 0){
            (*_ptrInstrumentController).SetPitchBend(i, pitchBend);
        }
    }
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
        _currentInstrument = _currentChannel;
        return _currentInstrument;


    case(RoundRobin):

        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            _currentInstrument++;

            //Loop to first instrument if end is reached
            _currentInstrument = (_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : _currentInstrument;

            //Check if valid instrument
            if(_instruments & (1 << _currentInstrument) != 0){
                return _currentInstrument;
            }
        }
        return _currentInstrument;


    case(Accending):

        for(int i = 0; (i < MAX_NUM_INSTRUMENTS); i++){
            if(_instruments & (1 << i) != 0)
            {
                uint8_t activeNotes = (*_ptrInstrumentController).getNumActiveNotes(i);

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

    case(Descending):

        for(int i = 31; (i >= 0); i--){
            if(_instruments & (1 << i) != 0)
            {
                uint8_t activeNotes = (*_ptrInstrumentController).getNumActiveNotes(i);

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

//returns -1 if no instrument found
uint8_t Distributor::CheckForNote(uint8_t note)
{
    uint8_t nextInstrument = _currentInstrument;

    switch(_distributionMethod){

    case(StrightThrough):
        if((*_ptrInstrumentController).isNoteActive(_currentChannel, note)){
            return _currentChannel;
        }
        return NONE;

    case(RoundRobin):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            nextInstrument--;

            //Only works bc 255 % 32 = 31
            nextInstrument = nextInstrument % MAX_NUM_INSTRUMENTS;

            //Check if valid instrument
            if(_instruments & (1 << nextInstrument) != 0){
                if((*_ptrInstrumentController).isNoteActive(nextInstrument, note)){
                    return nextInstrument;
                }
            }
        }
        return NONE;

    case(Accending):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){

            //Check if valid instrument
            if(_instruments & (1 << i) != 0){
                if((*_ptrInstrumentController).isNoteActive(i, note)){
                    return i;
                }
            }
        }
        return NONE;

    case(Descending):
        for(int i = 31; i >= 0; i--){

            //Check if valid instrument
            if(_instruments & (1 << i) != 0){
                if((*_ptrInstrumentController).isNoteActive(i, note)){
                    return i;
                }
            }
        }
        return NONE;
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

void Distributor::setDistributionMethod(DistributionMethod distribution){
    _distributionMethod = distribution;
}

void Distributor::setDamperPedal(bool damper){
    _damperPedal = damper;
}

void Distributor::setPolyphonic(bool polyphonic){
    _polyphonic = polyphonic;
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