/* 
 * Distributor.cpp
 *
 * Distributors serve to route midi notes to various instrument groups
 * via configurable algorithms. Each Algorithm accounts for the number of 
 * active notes playing on each instrument and the order of instruments 
 * to be played.
 *  
 * ex. (Midi ch 2 & 3) -> (instruments 4-8) Mapped via RoundRobin.
 * ex. (Midi ch 1,3,4) -> (instruments 1-4,7,8) Mapped via Ascending.
 * 
 */

#include "Distributor.h"
#include "Device.h"


Distributor::Distributor(InstrumentController* ptrInstrumentController)
{
    m_ptrInstrumentController = ptrInstrumentController;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Message Processor
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::processMessage(MidiMessage message)
{
    m_currentChannel = message.channel();

    //Determine Type of MIDI Msg and Call Associated Event
    switch(message.type()){

    case(MIDI_NoteOff):
        noteOffEvent(message.buffer[1],message.buffer[2]);
        break; 
    case(MIDI_NoteOn):
        noteOnEvent(message.buffer[1],message.buffer[2],message.channel());
        break;
    case(MIDI_KeyPressure):
        keyPressureEvent(message.buffer[1],message.buffer[2]);
        break;
    case(MIDI_ControlChange):
        controlChangeEvent(message.buffer[1],message.buffer[2]);
        break;
    case(MIDI_ProgramChange):
        programChangeEvent(message.buffer[1]);
        break;
    case(MIDI_ChannelPressure):
        channelPressureEvent(message.buffer[1]);
        break;
    case(MIDI_PitchBend):
        pitchBendEvent((message.buffer[1] << 7 | message.buffer[2]));
        break;
    case(MIDI_SysCommon):
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Event Handlers
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::noteOffEvent(uint8_t Note, uint8_t Velocity)
{
    //Find the first Instrument playing the given Note and Stop it.
    uint8_t instrument = checkForNote(Note);
    if(instrument != NONE){
        (*m_ptrInstrumentController).stopNote(instrument, Note, Velocity);
    }
}

void Distributor::noteOnEvent(uint8_t Note, uint8_t Velocity, uint8_t channel)
{
    //Check if note has 0 velocity representing a note off event
    if((Velocity == 0)){
        noteOffEvent(Note, Velocity);
        return;
    }

    //Get Next Instument Based on Distribution Method and Play Note
    uint8_t instrument = nextInstrument();
    if(instrument != NONE){ 
        (*m_ptrInstrumentController).playNote(instrument, Note, Velocity, channel);
    }
}

void Distributor::keyPressureEvent(uint8_t Note, uint8_t Velocity)
{
    //Find the First Active Instrument playing said Note and update its Velocity
    uint8_t instrument = checkForNote(Note);
    if(instrument != NONE){
        (*m_ptrInstrumentController).setKeyPressure(instrument, Note, Velocity);
    }
}

void Distributor::controlChangeEvent(uint8_t Controller, uint8_t Value)
{
    //Implemented in MessageHandler
}

void Distributor::programChangeEvent(uint8_t Program)
{
    //Not Yet Implemented
}

void Distributor::channelPressureEvent(uint8_t Velocity)
{
    //Not Yet Implemented
}

void Distributor::pitchBendEvent(uint16_t pitchBend)
{
    //Update Each Instruments Pitch Bend Value
    for(uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++){
        if((m_instruments & (1 << i)) != 0){
            (*m_ptrInstrumentController).setPitchBend(i, pitchBend);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns the instument ID of the next instrument to be played.
uint8_t Distributor::nextInstrument()
{
    bool validInsturment = false;
    uint8_t insturmentLeastActive = 0;
    uint8_t leastActiveNotes = 255;

    switch(m_distributionMethod){

    case(DistributionMethod::StraightThrough):
        
        // Return the Instument ID matching the current Messages Channel ID
        m_currentInstrument = m_currentChannel;
        if(!distributorHasInstrument(m_currentInstrument)) return m_currentInstrument;
        return NONE;

    case(DistributionMethod::RoundRobin):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            m_currentInstrument++;

            //Loop to first instrument if end is reached
            m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

            //Check if valid instrument
            if(!distributorHasInstrument(m_currentInstrument)) continue;
            return m_currentInstrument;
        }
        return NONE;
        

    case(DistributionMethod::RoundRobinBalance):

        insturmentLeastActive = NONE;
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            // Increment current Instrument
            m_currentInstrument++;

            // Loop to first instrument if end is reached
            m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

            // Check if valid instrument
            if(!distributorHasInstrument(m_currentInstrument)) continue;

            // If no active notes this must be the least active Instrument return
            uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(m_currentInstrument);
            if(activeNotes == 0) return m_currentInstrument;

            // Set this to Least Active Instrument if instrumentLeastActive is not yet set.
            if(insturmentLeastActive == NONE){
                insturmentLeastActive = m_currentInstrument;
                continue;
            }

            // Update the Least Active Instrument if needed.
            if(activeNotes < (*m_ptrInstrumentController).getNumActiveNotes(insturmentLeastActive)){
                insturmentLeastActive = m_currentInstrument;
            }
        }
        return insturmentLeastActive;


    case(DistributionMethod::Ascending):

        for(int i = 0; (i < MAX_NUM_INSTRUMENTS); i++){

            // Check if valid instrument
            if(!distributorHasInstrument(i)) continue;
            validInsturment = true;

            uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

            if(activeNotes < leastActiveNotes)
            {
                leastActiveNotes = activeNotes;
                insturmentLeastActive = i;
            }

            if(activeNotes == 0) return insturmentLeastActive;
        }
        return validInsturment ? m_currentInstrument : NONE;

    case(DistributionMethod::Descending):

        for(int i = (MAX_NUM_INSTRUMENTS - 1); (i >= 0); i--){
        
            // Check if valid instrument
            if(!distributorHasInstrument(i)) continue;
            validInsturment = true;
            
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

uint8_t Distributor::checkForNote(uint8_t note)
{
    uint8_t nextInstrument = m_currentInstrument;

    switch(m_distributionMethod){

    case(DistributionMethod::StraightThrough):
        if((*m_ptrInstrumentController).isNoteActive(m_currentChannel, note)){
            return m_currentChannel;
        }
        break;

    case(DistributionMethod::RoundRobin):
    case(DistributionMethod::RoundRobinBalance):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){
            //Decrement Instrument with Underflow Protection
            if(nextInstrument == 0) nextInstrument = MAX_NUM_INSTRUMENTS;
            nextInstrument--;

            //Check if valid instrument
            if(!distributorHasInstrument(nextInstrument)) continue;
            if((*m_ptrInstrumentController).isNoteActive(nextInstrument, note)) return nextInstrument;
        }
        break;

    case(DistributionMethod::Ascending):
        for(int i = 0; i < MAX_NUM_INSTRUMENTS; i++){

            //Check if valid instrument
            if(!distributorHasInstrument(i)) continue;
            if((*m_ptrInstrumentController).isNoteActive(i, note)) return i;
        }
        break;

    case(DistributionMethod::Descending):
        for(int i = (MAX_NUM_INSTRUMENTS - 1); i >= 0; i--){

            //Check if valid instrument
            if(!distributorHasInstrument(i)) continue;
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

bool Distributor::distributorHasInstrument(int instrumentId){
    return ((m_instruments & (1 << instrumentId)) != 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Getters
////////////////////////////////////////////////////////////////////////////////////////////////////

std::array<uint8_t,NUM_DISTRIBUTOR_CFG_BYTES> Distributor::toSerial()
{
    std::array<std::uint8_t,NUM_DISTRIBUTOR_CFG_BYTES> distributorObj;

    uint8_t distributorBoolByte = 0;
    if(this->m_damperPedal)   distributorBoolByte |= (1 << 0);
    if(this->m_polyphonic)    distributorBoolByte |= (1 << 1);
    if(this->m_noteOverwrite) distributorBoolByte |= (1 << 2);


    //Cast Distributor Construct to uint8_t Array every MSB = 0 as per the Midi Protocal
    // (https://docs.google.com/spreadsheets/d/1AgS2-iZVLSL0w_MafbeReRx4u_9m_e4OTCsIhKC-QMg/edit?usp=sharing)
    distributorObj[0] = 0; //Distributor ID MSB
    distributorObj[1] = 0; //Distributor ID LSB
    distributorObj[2] = static_cast<uint8_t>((m_channels >> 14) & 0x03);
    distributorObj[3] = static_cast<uint8_t>((m_channels >> 7) & 0x7F);
    distributorObj[4] = static_cast<uint8_t>((m_channels >> 0) & 0x7F);
    distributorObj[5] = static_cast<uint8_t>((m_instruments >> 28) & 0x0F);
    distributorObj[6] = static_cast<uint8_t>((m_instruments >> 21) & 0x7F);
    distributorObj[7] = static_cast<uint8_t>((m_instruments >> 14) & 0x7F);
    distributorObj[8] = static_cast<uint8_t>((m_instruments >> 7) & 0x7F);
    distributorObj[9] = static_cast<uint8_t>((m_instruments >> 0) & 0x7F);
    distributorObj[10] = static_cast<uint8_t>(m_distributionMethod);
    distributorObj[11] = distributorBoolByte;
    distributorObj[12] = m_minNote;
    distributorObj[13] = m_maxNote;
    distributorObj[14] = m_numPolyphonicNotes;
    distributorObj[15] = 0; //Reserved

    return distributorObj;

    //Usefull Idea
    //memcpy(&distributorObj[4], &m_channels, 2);
    //memcpy(&distributorObj[6], &m_instruments, 4);
}

uint16_t Distributor::getChannels(){
    return m_channels;
}

uint32_t Distributor::getInstruments(){
    return m_instruments;
}

void Distributor::setDistributionMethod(DistributionMethod distribution){
    m_distributionMethod = distribution;
}

void Distributor::setDamperPedal(bool damper){
    m_damperPedal = damper;
}

void Distributor::setPolyphonic(bool polyphonic){
    m_polyphonic = polyphonic;
}

void Distributor::setNoteOverwrite(bool noteOverwrite){
    m_noteOverwrite = noteOverwrite;
}

void Distributor::setMinMaxNote(uint8_t minNote, uint8_t maxNote){
    m_minNote = minNote;
    m_maxNote = maxNote;
}

void Distributor::setNumPolyphonicNotes(uint8_t numPolyphonicNotes){
    m_numPolyphonicNotes = numPolyphonicNotes;
}
    
void Distributor::setChannels(uint16_t channels){
    m_channels = channels;
}
    
void Distributor::setInstruments(uint32_t instruments){
    m_instruments = instruments;
}