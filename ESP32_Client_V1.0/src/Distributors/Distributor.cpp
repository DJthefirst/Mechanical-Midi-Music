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
#include "DistributionStrategies.h"

Distributor::Distributor(InstrumentController* ptrInstrumentController)
{
    m_ptrInstrumentController = ptrInstrumentController;
    // Initialize with default strategy
    updateDistributionStrategy();
}

Distributor::~Distributor()
{
    // Clear this distributor from all instruments to prevent hanging notes
    if (m_ptrInstrumentController) {
        m_ptrInstrumentController->clearDistributorFromInstrument(this);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Message Processor
////////////////////////////////////////////////////////////////////////////////////////////////////

//Determine Type of MIDI Msg and Call Associated Event
void Distributor::processMessage(MidiMessage message){
    m_currentChannel = message.channel();

    // Check if this distributor handles this channel
    if (!channelEnabled(m_currentChannel)) return;

    // Check if muted
    if(m_muted)return;

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
        // controlChangeEvent(message.buffer[1],message.buffer[2]); //Implemented in MessageHandler
        break;
    case(MIDI_ProgramChange):
        
        programChangeEvent(message.buffer[1]);
        break;
    case(MIDI_ChannelPressure):
        channelPressureEvent(message.buffer[1]);
        break;
    case(MIDI_PitchBend):
        pitchBendEvent((static_cast<uint16_t>(message.buffer[2]) << 7) | static_cast<uint16_t>(message.buffer[1]), m_currentChannel);
        break;
    case(MIDI_SysCommon):
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Event Handlers
////////////////////////////////////////////////////////////////////////////////////////////////////

// Find the first instrument playing the given note and stop it
void Distributor::noteOffEvent(uint8_t Note, uint8_t Velocity)
{
    const uint8_t instrument = checkForNote(Note);
    if(instrument != NONE){
        m_ptrInstrumentController->stopNote(instrument, Note, Velocity);
    }
}

// Get next instrument based on distribution method and play note
void Distributor::noteOnEvent(uint8_t Note, uint8_t Velocity, uint8_t channel)
{
    // Check if note has 0 velocity representing a note off event
    if(Velocity == 0){
        noteOffEvent(Note, Velocity);
        return;
    }

    // Get next instrument based on distribution method and play note
    const uint8_t instrument = nextInstrument();
    if(instrument != NONE){ 
        // Use the new overloaded method that tracks the distributor
        m_ptrInstrumentController->playNote(instrument, Note, Velocity, channel, this);
    }
}

// Find the first active instrument playing said note and update its velocity
void Distributor::keyPressureEvent(uint8_t Note, uint8_t Velocity)
{
    const uint8_t instrument = checkForNote(Note);
    if(instrument != NONE){
        m_ptrInstrumentController->setKeyPressure(instrument, Note, Velocity);
    }
}

void Distributor::programChangeEvent(uint8_t Program)
{
    // Not yet implemented
    m_ptrInstrumentController->resetAll();
}

void Distributor::channelPressureEvent(uint8_t Velocity)
{
    // Not yet implemented 
}

// Update each instrument's pitch bend value
void Distributor::pitchBendEvent(uint16_t pitchBend, uint8_t channel)
{
    // Optimize: use bit manipulation to iterate only over set bits
    uint32_t instrumentMask = m_instruments;
    for(uint8_t i = 0; instrumentMask != 0 && i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        if(instrumentMask & 1){
            m_ptrInstrumentController->setPitchBend(i, pitchBend, channel);
        }
        instrumentMask >>= 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Routing Logic
////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns the instument ID of the next instrument to be played.
uint8_t Distributor::nextInstrument()
{
    if (m_distributionStrategy) {
        return m_distributionStrategy->getNextInstrument(
            m_ptrInstrumentController, 
            m_currentInstrument, 
            m_instruments
        );
    }
    return NONE;
}

//Returns the instument which the first note is played or NONE.
//*Note this function is optimised for nonpolyphonic playback and the Distribution method.
uint8_t Distributor::checkForNote(uint8_t note)
{
    uint8_t instrument = m_currentInstrument;

    switch(m_distributionMethod){

    // Check for note being played on the instrument in the current channel position
    case(DistributionMethod::StraightThrough):
        if((*m_ptrInstrumentController).isNoteActive(m_currentChannel, note)){
            return m_currentChannel;
        }
        break;

    // Check for note being played on the instrument iterating backwards through all instruments
    case(DistributionMethod::RoundRobin):
    case(DistributionMethod::RoundRobinBalance):
        // Iterate through each instrument in reverse
        for(int i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
            // Decrement instrument with underflow protection
            if(instrument == 0) instrument = HardwareConfig::MAX_NUM_INSTRUMENTS;
            --instrument;

            // Check if valid instrument
            if(!distributorHasInstrument(instrument)) continue;
            if(m_ptrInstrumentController->isNoteActive(instrument, note)) return instrument;
        }
        break;

    // Check for note being played on the instrument starting at instrument 0
    case(DistributionMethod::Ascending):
        // Optimize: iterate only over instruments in our mask
        {
            uint32_t instrumentMask = m_instruments;
            for(uint8_t i = 0; instrumentMask != 0 && i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
                if((instrumentMask & 1) && m_ptrInstrumentController->isNoteActive(i, note)){
                    return i;
                }
                instrumentMask >>= 1;
            }
        }
        break;

    // Check for note being played on the instrument starting at the last instrument iterating in reverse
    case(DistributionMethod::Descending):
        for(int i = (HardwareConfig::MAX_NUM_INSTRUMENTS - 1); i >= 0; --i){
            // Check if valid instrument
            if(!distributorHasInstrument(i)) continue;
            if(m_ptrInstrumentController->isNoteActive(i, note)) return i;
        }
        break;

    default:
        // TODO: Handle error
        break;
    }
    return NONE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns true if a distributor contains the designated instrument in its pool
constexpr bool Distributor::distributorHasInstrument(int instrumentId) const noexcept {
    return (instrumentId >= 0 && instrumentId < 32) && ((m_instruments & (1U << instrumentId)) != 0);
}

bool Distributor::channelEnabled(uint8_t channel){
    if (channel >= 16) return false;
    return (m_channels & (1 << channel)) != 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Getters
////////////////////////////////////////////////////////////////////////////////////////////////////

//Returns Distributor construct without ID
std::array<uint8_t,DISTRIBUTOR_NUM_CFG_BYTES> Distributor::toSerial()
{
    std::array<std::uint8_t,DISTRIBUTOR_NUM_CFG_BYTES> distributorObj = {}; // Initialize all bytes to 0

    uint8_t distributorBoolByte = 0;
    if(this->m_muted)         distributorBoolByte |= DISTRIBUTOR_BOOL_MUTED;
    if(this->m_damperPedal)   distributorBoolByte |= DISTRIBUTOR_BOOL_DAMPERPEDAL;
    if(this->m_polyphonic)    distributorBoolByte |= DISTRIBUTOR_BOOL_POLYPHONIC;
    if(this->m_noteOverwrite) distributorBoolByte |= DISTRIBUTOR_BOOL_NOTEOVERWRITE;


    //Cast Distributor Construct to uint8_t Array every MSB = 0 as per the Midi Protocal
    // (https://docs.google.com/spreadsheets/d/1AgS2-iZVLSL0w_MafbeReRx4u_9m_e4OTCsIhKC-QMg/edit?usp=sharing)
    distributorObj[0] = 0; //Distributor ID MSB Generated in MsgHandler
    distributorObj[1] = 0; //Distributor ID LSB Generated in MsgHandler
    distributorObj[2] = static_cast<uint8_t>((m_channels >> 14) & 0x03);
    distributorObj[3] = static_cast<uint8_t>((m_channels >> 7) & 0x7F);
    distributorObj[4] = static_cast<uint8_t>((m_channels >> 0) & 0x7F);
    distributorObj[5] = static_cast<uint8_t>((m_instruments >> 28) & 0x0F);
    distributorObj[6] = static_cast<uint8_t>((m_instruments >> 21) & 0x7F);
    distributorObj[7] = static_cast<uint8_t>((m_instruments >> 14) & 0x7F);
    distributorObj[8] = static_cast<uint8_t>((m_instruments >> 7) & 0x7F);
    distributorObj[9] = static_cast<uint8_t>((m_instruments >> 0) & 0x7F);
    distributorObj[10] = static_cast<uint8_t>(m_distributionMethod) & 0x7F;
    distributorObj[11] = distributorBoolByte & 0x7F;
    distributorObj[12] = m_minNote & 0x7F;
    distributorObj[13] = m_maxNote & 0x7F;
    distributorObj[14] = m_numPolyphonicNotes & 0x7F;
    distributorObj[15] = 0; //Reserved

    return distributorObj;
}

bool Distributor::getMuted() const {
    return m_muted;
}

//Returns Distributor Channels
uint16_t Distributor::getChannels() const {
    return m_channels;
}

//Returns Distributor Channels
uint32_t Distributor::getInstruments() const {
    return m_instruments;
}

//Returns Distribution Method
DistributionMethod Distributor::getDistributionMethod() const {
    return m_distributionMethod;
}

//Distribute message to instruments
void Distributor::distributeMessage(MidiMessage& message) {
    processMessage(message);
}

//Configures Distributor from construct
void Distributor::setDistributor(uint8_t data[]){
    // Decode Distributor Construct
    uint16_t channels = 
          (data[2] << 14)
        | (data[3] << 7) 
        | (data[4] << 0);
    uint32_t instruments = 
          (data[5] << 28)
        | (data[6] << 21) 
        | (data[7] << 14)
        | (data[8] << 7)
        | (data[9] << 0);
    DistributionMethod distribMethod = static_cast<DistributionMethod>(data[10]);

    m_channels = channels; // 1
    m_instruments = instruments; // 1,2
    m_distributionMethod = distribMethod;
    m_muted = (data[11] & DISTRIBUTOR_BOOL_MUTED) != 0;
    m_damperPedal = (data[11] & DISTRIBUTOR_BOOL_DAMPERPEDAL) != 0;
    m_polyphonic = (data[11] & DISTRIBUTOR_BOOL_POLYPHONIC) != 0;
    m_noteOverwrite = (data[11] & DISTRIBUTOR_BOOL_NOTEOVERWRITE) != 0;
    m_minNote = data[12];
    m_maxNote = data[13];
    m_numPolyphonicNotes = (data[14]);
}

//Configures Distributor Distribution Method
void Distributor::setDistributionMethod(DistributionMethod distribution){
    m_distributionMethod = distribution;
    updateDistributionStrategy();
}

void Distributor::toggleMuted(){
    m_muted = !m_muted;
}

//Configures Distributor Boolean
void Distributor::setMuted(bool muted){
    if(m_muted != muted) (*m_ptrInstrumentController).stopAll();
    m_muted = muted;
}

//Configures Distributor Damper Pedal
void Distributor::setDamperPedal(bool damper){
    m_damperPedal = damper;
}

//Configures Distributor Polphonic Notes
void Distributor::setPolyphonic(bool polyphonic){
    m_polyphonic = polyphonic;
}

//Configures Distributor Note Overwrite
void Distributor::setNoteOverwrite(bool noteOverwrite){
    m_noteOverwrite = noteOverwrite;
}

//Configures Distributor Minimum and Maximum Notes
void Distributor::setMinMaxNote(uint8_t minNote, uint8_t maxNote){
    m_minNote = minNote;
    m_maxNote = maxNote;
}

//Configures Distributor maximum number of Polyphonic notes
void Distributor::setNumPolyphonicNotes(uint8_t numPolyphonicNotes){
    m_numPolyphonicNotes = numPolyphonicNotes;
}

//Configures Distributor accepted MIDI channels
void Distributor::setChannels(uint16_t channels){
    m_channels = channels;
}
    
//Configures Distributor Instrument Pool
void Distributor::setInstruments(uint32_t instruments){
    m_instruments = instruments;
}

//Updates the distribution strategy based on the current method
void Distributor::updateDistributionStrategy(){
    switch(m_distributionMethod) {
        case DistributionMethod::RoundRobinBalance:
            m_distributionStrategy = std::make_unique<RoundRobinBalanceStrategy>();
            break;
        case DistributionMethod::RoundRobin:
            m_distributionStrategy = std::make_unique<RoundRobinStrategy>();
            break;
        case DistributionMethod::Ascending:
            m_distributionStrategy = std::make_unique<AscendingStrategy>();
            break;
        case DistributionMethod::Descending:
            m_distributionStrategy = std::make_unique<DescendingStrategy>();
            break;
        case DistributionMethod::StraightThrough:
            m_distributionStrategy = std::make_unique<StraightThroughStrategy>();
            break;
        default:
            // Fallback to RoundRobinBalance as default
            m_distributionStrategy = std::make_unique<RoundRobinBalanceStrategy>();
            break;
    }
}