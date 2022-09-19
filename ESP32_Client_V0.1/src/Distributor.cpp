/* 
 * Distributor.cpp
 *
 * Distributors serve to route midi notes to varrious instrument groups. ex. Midi ch 2 & 3 -> instruments 4-8
 *
 */

#include "Distributor.h"

Distributor::Distributor(InstrumentController* ptrInstrumentController)
{
    m_ptrInstrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Message Processor
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::ProcessMessage(uint8_t message[])
{
    m_currentChannel = (message[0] & 0b00001111);

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
        PitchBendEvent((message[1] | message[2] << 7));
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
        (*m_ptrInstrumentController).StopNote(instrument, Note, Velocity);
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
            (*m_ptrInstrumentController).PlayNote(instrument, Note, Velocity);
        }
    }
}

void Distributor::KeyPressureEvent(uint8_t Note, uint8_t Velocity)
{
    uint8_t instrument = CheckForNote(Note);
    if(instrument != NONE){
        (*m_ptrInstrumentController).SetKeyPressure(instrument, Note, Velocity);
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
        if((m_instruments & (1 << i)) != 0){
            (*m_ptrInstrumentController).SetPitchBend(i, pitchBend);
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

    switch(m_distributionMethod){

    case(StraightThrough):
        m_currentInstrument = m_currentChannel;
        return m_currentInstrument;


    case(RoundRobin):

        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            m_currentInstrument++;

            //Loop to first instrument if end is reached
            m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

            //Check if valid instrument
            if(m_instruments & (1 << m_currentInstrument) != 0){
                return m_currentInstrument;
            }
        }
        return m_currentInstrument;


    case(Accending):

        for(int i = 0; (i < MAX_NUM_INSTRUMENTS); i++){
            if(m_instruments & (1 << i) != 0)
            {
                uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

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
            if(m_instruments & (1 << i) != 0)
            {
                uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

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
    uint8_t nextInstrument = m_currentInstrument;

    switch(m_distributionMethod){

    case(StraightThrough):
        if((*m_ptrInstrumentController).isNoteActive(m_currentChannel, note)){
            return m_currentChannel;
        }
        return NONE;

    case(RoundRobin):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            nextInstrument--;

            //Only works bc 255 % 32 = 31
            nextInstrument = nextInstrument % MAX_NUM_INSTRUMENTS;

            //Check if valid instrument
            if(m_instruments & (1 << nextInstrument) != 0){
                if((*m_ptrInstrumentController).isNoteActive(nextInstrument, note)){
                    return nextInstrument;
                }
            }
        }
        return NONE;

    case(Accending):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){

            //Check if valid instrument
            if(m_instruments & (1 << i) != 0){
                if((*m_ptrInstrumentController).isNoteActive(i, note)){
                    return i;
                }
            }
        }
        return NONE;

    case(Descending):
        for(int i = 31; i >= 0; i--){

            //Check if valid instrument
            if(m_instruments & (1 << i) != 0){
                if((*m_ptrInstrumentController).isNoteActive(i, note)){
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
void Distributor::GetDistributor(uint8_t* outputArray)
{
    uint8_t dstributorObj[] = {((m_noteOverwrite << 7) & (m_numPolyphonicNotes)), m_minNote, m_maxNote, m_channels >> 8, m_channels, 
        m_instruments >> 24, m_instruments >> 16, m_instruments >> 8, m_instruments};
    outputArray = dstributorObj;
}

uint16_t Distributor::GetChannels(){
    return m_channels;
}

void Distributor::SetDistributionMethod(DistributionMethod distribution){
    m_distributionMethod = distribution;
}

void Distributor::SetDamperPedal(bool damper){
    m_damperPedal = damper;
}

void Distributor::SetPolyphonic(bool polyphonic){
    m_polyphonic = polyphonic;
}

void Distributor::SetNoteOverwrite(bool noteOverwrite)
{
    m_noteOverwrite = noteOverwrite;
}

void Distributor::SetMinMaxNote(uint8_t minNote, uint8_t maxNote)
{
    m_minNote = minNote;
    m_maxNote = maxNote;
}

void Distributor::SetNumPolyphonicNotes(uint8_t numPolyphonicNotes)
{
    m_numPolyphonicNotes = numPolyphonicNotes;
}
    
void Distributor::SetChannels(uint16_t channels)
{
    m_channels = channels;
}
    
void Distributor::SetInstruments(uint32_t instruments)
{
    m_instruments = instruments;
}