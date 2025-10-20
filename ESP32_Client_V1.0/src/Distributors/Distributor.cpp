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
#include "../Instruments/InstrumentController.h"
#include "../Utility/Utility.h"

Distributor::Distributor()
{
    m_instrumentController = InstrumentController<InstrumentType>::getInstance();
    // Initialize with default strategy
    updateDistributionStrategy();
}

Distributor::~Distributor(){
    // Make sure to stop any active notes when the distributor is destroyed
    if (m_instrumentController) {
        stopActiveNotes();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Message Processor
////////////////////////////////////////////////////////////////////////////////////////////////////

//Determine Type of MIDI Msg and Call Associated Event
void Distributor::processMessage(MidiMessage message){
    uint8_t currentChannel = message.channel();

    // Check if this distributor handles this channel
    if (!channelEnabled(currentChannel)) return;

    // Check if muted
    if(m_muted)return;

    switch(message.type()){

    case(Midi::NoteOff):
        noteOffEvent(message.buffer[1],message.buffer[2],message.channel());
        break; 
    case(Midi::NoteOn):
        noteOnEvent(message.buffer[1],message.buffer[2],message.channel());
        break;
    case(Midi::KeyPressure):
        keyPressureEvent(message.buffer[1],message.buffer[2]);
        break;
    case(Midi::ControlChange):
        // controlChangeEvent(message.buffer[1],message.buffer[2]); //Implemented in MessageHandler
        break;
    case(Midi::ProgramChange):
        
        programChangeEvent(message.buffer[1]);
        break;
    case(Midi::ChannelPressure):
        channelPressureEvent(message.buffer[1]);
        break;
    case(Midi::PitchBend):
        pitchBendEvent((static_cast<uint16_t>(message.buffer[2]) << 7) | static_cast<uint16_t>(message.buffer[1]), currentChannel);
        break;
    case(Midi::SysCommon):
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Event Handlers
////////////////////////////////////////////////////////////////////////////////////////////////////

// Find the first instrument playing the given note and stop it
void Distributor::noteOffEvent(uint8_t note, uint8_t velocity, uint8_t channel)
{
    m_distributionStrategy->stopActiveInstrument(note, velocity, channel);
}

// Get next instrument based on distribution method and play note
void Distributor::noteOnEvent(uint8_t note, uint8_t velocity, uint8_t channel)
{

    if (note < m_minNote || note > m_maxNote) {
        return;
    }

    // Check if note has 0 velocity representing a note off event
    if(velocity == 0){
        m_distributionStrategy->stopActiveInstrument(note, velocity, channel);
    }else{
        m_distributionStrategy->playNextInstrument(note, velocity, channel);
    }
}

// Find the first active instrument playing said note and update its velocity
void Distributor::keyPressureEvent(uint8_t Note, uint8_t Velocity)
{
    //TODO: Re-implement distributor tracking
    // const uint8_t instrument = checkForNote(Note);
    // if(instrument != NONE){
    //     m_ptrInstrumentController->setKeyPressure(instrument, Note, Velocity);
    // }
}

void Distributor::programChangeEvent(uint8_t Program)
{
    // Not yet implemented
    m_instrumentController->resetAll();
}

void Distributor::channelPressureEvent(uint8_t Velocity)
{
    // Not yet implemented 
}

// Update each instrument's pitch bend value
void Distributor::pitchBendEvent(uint16_t pitchBend, uint8_t channel)
{
    // Iterate over enabled instruments
    for(uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i){
        if(m_instruments[i]){
            m_instrumentController->setPitchBend(i, pitchBend, channel);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns true if a distributor contains the designated instrument in its pool
// (Implementation moved to end of file)

bool Distributor::channelEnabled(uint8_t channel){
    if (channel >= 16) return false;
    return m_channels[channel];
}

// Helper function to check if distributor handles the given instrument
bool Distributor::instrumentEnabled(int instrumentId) const noexcept {
    if (instrumentId < 0 || instrumentId >= NUM_Instruments) {
        return false;
    }
    return m_instruments.test(instrumentId);
}

void Distributor::stopActiveNotes() {
    // Iterate through all possible instruments and if this distributor
    // is recorded as the last distributor for that instrument, stop the
    // note to avoid hanging notes and clear the tracking pointer.

    for (uint8_t i = 0; i < HardwareConfig::MAX_NUM_INSTRUMENTS; ++i) {
        void* last = m_instrumentController->getLastDistributor(i);
        if (last == static_cast<void*>(this)) {
            // If instrument is active, stop it. Use stopNote which will
            // also clear the _lastDistributor entry in most implementations.
            if (m_instrumentController->getNumActiveNotes(i) != 0) {
                m_instrumentController->stopNote(i, 0);
            } else {
                // Ensure the distributor tracking is cleared even if not active
                m_instrumentController->setLastDistributor(i, nullptr, NONE);
            }
        }
    }
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
    distributorObj[2] = (Utility::bitsetToBytes(m_channels)[2] & 0x03);
    distributorObj[3] = (Utility::bitsetToBytes(m_channels)[1] & 0x7F);
    distributorObj[4] = (Utility::bitsetToBytes(m_channels)[0] & 0x7F);
    distributorObj[5] = (Utility::bitsetToBytes(m_instruments)[4] & 0x0F);
    distributorObj[6] = (Utility::bitsetToBytes(m_instruments)[3] & 0x7F);
    distributorObj[7] = (Utility::bitsetToBytes(m_instruments)[2] & 0x7F);
    distributorObj[8] = (Utility::bitsetToBytes(m_instruments)[1] & 0x7F);
    distributorObj[9] = (Utility::bitsetToBytes(m_instruments)[0] & 0x7F);
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
std::bitset<NUM_Channels> Distributor::getChannels() const {
    return m_channels;
}

//Returns Distributor Channels
std::bitset<NUM_Instruments> Distributor::getInstruments() const {
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

    m_channels = std::bitset<NUM_Channels>(channels);
    m_instruments = std::bitset<NUM_Instruments>(instruments);
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
    stopActiveNotes();
    m_distributionMethod = distribution;
    updateDistributionStrategy();
}

void Distributor::toggleMuted(){
    stopActiveNotes();
    m_muted = !m_muted;
}

//Configures Distributor Boolean
void Distributor::setMuted(bool muted){
    if(m_muted != muted) stopActiveNotes();
    m_muted = muted;
}

//Configures Distributor Damper Pedal
void Distributor::setDamperPedal(bool damper){
    stopActiveNotes();
    m_damperPedal = damper;
}

//Configures Distributor Polphonic Notes
void Distributor::setPolyphonic(bool polyphonic){
    stopActiveNotes();
    m_polyphonic = polyphonic;
}

//Configures Distributor Note Overwrite
void Distributor::setNoteOverwrite(bool noteOverwrite){
    stopActiveNotes();
    m_noteOverwrite = noteOverwrite;
}

//Configures Distributor Minimum and Maximum Notes
void Distributor::setMinMaxNote(uint8_t minNote, uint8_t maxNote){
    stopActiveNotes();
    m_minNote = minNote;
    m_maxNote = maxNote;
}

//Configures Distributor maximum number of Polyphonic notes
void Distributor::setNumPolyphonicNotes(uint8_t numPolyphonicNotes){
    stopActiveNotes();
    m_numPolyphonicNotes = numPolyphonicNotes;
}

//Configures Distributor accepted MIDI channels
void Distributor::setChannels(std::bitset<NUM_Channels> channels){
    stopActiveNotes();
    m_channels = channels;
}
    
//Configures Distributor Instrument Pool
void Distributor::setInstruments(std::bitset<NUM_Instruments> instruments){
    stopActiveNotes();
    m_instruments = instruments;
}

//Updates the distribution strategy based on the current method
void Distributor::updateDistributionStrategy(){
    stopActiveNotes();
    switch(m_distributionMethod) {
        case DistributionMethod::RoundRobinBalance:
            m_distributionStrategy = std::make_unique<RoundRobinBalanceStrategy>(this);
            break;
        case DistributionMethod::RoundRobin:
            m_distributionStrategy = std::make_unique<RoundRobinStrategy>(this);
            break;
        case DistributionMethod::Ascending:
            m_distributionStrategy = std::make_unique<AscendingStrategy>(this);
            break;
        case DistributionMethod::Descending:
            m_distributionStrategy = std::make_unique<DescendingStrategy>(this);
            break;
        case DistributionMethod::StraightThrough:
            m_distributionStrategy = std::make_unique<StraightThroughStrategy>(this);
            break;
        default:
            // Fallback to RoundRobinBalance as default
            m_distributionStrategy = std::make_unique<RoundRobinBalanceStrategy>(this);
            break;
    }
}