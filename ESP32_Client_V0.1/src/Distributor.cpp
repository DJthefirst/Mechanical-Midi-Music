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
    bool validInsturment = false;
    uint8_t insturmentLeastActive = 0;
    uint8_t leastActiveNotes = 255;

    switch(m_distributionMethod){

    case(StraightThrough):
        
        m_currentInstrument = m_currentChannel;
        if(!DistributorHasInstrument(m_currentInstrument)) return m_currentInstrument;
        return NONE;

    case(RoundRobin):
;
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            m_currentInstrument++;

            //Loop to first instrument if end is reached
            m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

            //Check if valid instrument
            if(!((m_instruments & (1 << m_currentInstrument)) != 0)) continue;
            return validInsturment;
        }
        return NONE;
        

    case(RoundRobinBalance):

        insturmentLeastActive = NONE;
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            m_currentInstrument++;

            //Loop to first instrument if end is reached
            m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

            //Check if valid instrument
            if(!DistributorHasInstrument(m_currentInstrument)) continue;
            validInsturment = true;

            uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(m_currentInstrument);
            if(activeNotes == 0) return m_currentInstrument;

            if(insturmentLeastActive == NONE){
                insturmentLeastActive = m_currentInstrument;
                continue;
            }

            if(activeNotes < (*m_ptrInstrumentController).getNumActiveNotes(insturmentLeastActive)){
                insturmentLeastActive = m_currentInstrument;
            }
        }
        return insturmentLeastActive;


    case(Ascending):

        for(int i = 0; (i < MAX_NUM_INSTRUMENTS); i++){

            //Check if valid instrument
            if(!DistributorHasInstrument(i)) continue;
            bool validInsturment = true;

            uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

            if(activeNotes < leastActiveNotes)
            {
                leastActiveNotes = activeNotes;
                insturmentLeastActive = i;
            }

            if(activeNotes == 0) return insturmentLeastActive;
        }
        return validInsturment ? m_currentInstrument : NONE;

    case(Descending):

        for(int i = (MAX_NUM_INSTRUMENTS - 1); (i >= 0); i--){
        
            //Check if valid instrument
            if(!DistributorHasInstrument(i)) continue;
            bool validInsturment = true;
            
            uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

            if(activeNotes == 0) return insturmentLeastActive;

            if(activeNotes < leastActiveNotes)
            {
                leastActiveNotes = activeNotes;
                insturmentLeastActive = i;
            }
        
        }
        return validInsturment ? m_currentInstrument : NONE;

    default:
        //TODO Handle Error
        return NONE;
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
        break;

    case(RoundRobin,RoundRobinBalance):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            
            //Decrement Instrument with Underflow Protection
            nextInstrument--;
            if(nextInstrument == (uint8_t)-1) nextInstrument = MAX_NUM_INSTRUMENTS - 1;

            //Check if valid instrument
            if(!DistributorHasInstrument(nextInstrument)) continue;
            if((*m_ptrInstrumentController).isNoteActive(nextInstrument, note)) return nextInstrument;
        }
        break;

    case(Ascending):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){

            //Check if valid instrument
            if(!DistributorHasInstrument(i)) continue;
            if((*m_ptrInstrumentController).isNoteActive(i, note)) return i;
        }
        break;

    case(Descending):
        for(int i = (MAX_NUM_INSTRUMENTS - 1); i >= 0; i--){

            //Check if valid instrument
            if(!DistributorHasInstrument(i)) continue;
            if((*m_ptrInstrumentController).isNoteActive(i, note)) return i;
        }
        break;

    default:
        //TODO Handle Error
        break;
    }
    return NONE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

bool Distributor::DistributorHasInstrument(int instrumentId){
    return ((m_instruments & (1 << instrumentId)) != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters
////////////////////////////////////////////////////////////////////////////////////////////////////

//output Array = uint8_t[10]
uint8_t* Distributor::GetDistributor()
{
    uint8_t distributorObj[10];

    uint8_t distributorByte0 = 0;
    if(m_damperPedal)   distributorByte0 |= (1 << 0);
    if(m_polyphonic)    distributorByte0 |= (1 << 1);
    if(m_noteOverwrite) distributorByte0 |= (1 << 2);

    distributorObj[0] = distributorByte0;
    distributorObj[1] = m_numPolyphonicNotes;
    distributorObj[2] = m_minNote;
    distributorObj[3] = m_maxNote;
    distributorObj[4] = static_cast<uint8_t>( m_channels >> 0);
    distributorObj[5] = static_cast<uint8_t>( m_channels >> 8);
    distributorObj[6] = static_cast<uint8_t>( m_instruments >> 0);
    distributorObj[7] = static_cast<uint8_t>( m_instruments >> 8);
    distributorObj[8] = static_cast<uint8_t>( m_instruments >> 16);
    distributorObj[9] = static_cast<uint8_t>( m_instruments >> 24);
    //memcpy(&distributorObj[4], &m_channels, 2);
    //memcpy(&distributorObj[6], &m_instruments, 4);

    return distributorObj;
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