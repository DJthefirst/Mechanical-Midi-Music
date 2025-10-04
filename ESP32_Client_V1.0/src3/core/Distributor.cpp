/*
 * Distributor.cpp - V2.0 Runtime Distributor Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 */

#define USE_V2_CONFIG
#include "Distributor.h"
#include <Arduino.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////////////////////////////

Distributor::Distributor(InstrumentController* ptrInstrumentController)
    : m_ptrInstrumentController(ptrInstrumentController) {
    
    if (!m_ptrInstrumentController) {
        Serial.println("ERROR: Distributor created with null InstrumentController");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Core Processing
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::processMessage(const MidiMessage& message) {
    m_currentChannel = message.channel();

    // Check if this distributor handles this channel
    if (!channelEnabled(m_currentChannel)) return;
    
    // Check if muted
    if (m_muted) return;

    switch (message.type()) {
        case MIDI_NoteOff:
            noteOffEvent(message.data1, message.data2);
            break;
            
        case MIDI_NoteOn:
            noteOnEvent(message.data1, message.data2, message.channel());
            break;
            
        case MIDI_KeyPressure:
            keyPressureEvent(message.data1, message.data2);
            break;
            
        case MIDI_ProgramChange:
            programChangeEvent(message.data1);
            break;
            
        case MIDI_ChannelPressure:
            channelPressureEvent(message.data1);
            break;
            
        case MIDI_PitchBend:
            pitchBendEvent(message.pitchBend(), m_currentChannel);
            break;
            
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// MIDI Event Handlers
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::noteOnEvent(uint8_t note, uint8_t velocity, uint8_t channel) {
    // Note On with velocity 0 is Note Off
    if (velocity == 0) {
        noteOffEvent(note, velocity);
        return;
    }

    // Check note range
    if (!noteInRange(note)) return;

    // Get next instrument to play
    uint8_t instrument = nextInstrument();
    if (instrument != NONE) {
        m_ptrInstrumentController->playNote(instrument, note, velocity, channel);
    }
}

void Distributor::noteOffEvent(uint8_t note, uint8_t velocity) {
    // Find instrument playing this note
    uint8_t instrument = checkForNote(note);
    if (instrument != NONE) {
        m_ptrInstrumentController->stopNote(instrument, note, velocity);
    }
}

void Distributor::keyPressureEvent(uint8_t note, uint8_t velocity) {
    uint8_t instrument = checkForNote(note);
    if (instrument != NONE) {
        m_ptrInstrumentController->setKeyPressure(instrument, note, velocity);
    }
}

void Distributor::programChangeEvent(uint8_t program) {
    m_ptrInstrumentController->setProgramChange(program);
}

void Distributor::channelPressureEvent(uint8_t velocity) {
    m_ptrInstrumentController->setChannelPressure(velocity);
}

void Distributor::pitchBendEvent(uint16_t pitchBend, uint8_t channel) {
    // Apply pitch bend to all enabled instruments
    for (uint8_t i = 0; i < 32; i++) {
        if (distributorHasInstrument(i)) {
            m_ptrInstrumentController->setPitchBend(i, pitchBend, channel);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Routing Logic
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t Distributor::nextInstrument() {
    switch (m_distributionMethod) {
        case DistributionMethod::RoundRobinBalance:
            return nextInstrumentRoundRobinBalance();
        case DistributionMethod::RoundRobin:
            return nextInstrumentRoundRobin();
        case DistributionMethod::Sequential:
            return nextInstrumentSequential();
        case DistributionMethod::Random:
            return nextInstrumentRandom();
        case DistributionMethod::StraightThrough:
            return nextInstrumentStraightThrough();
        default:
            return NONE;
    }
}

uint8_t Distributor::nextInstrumentRoundRobin() {
    uint8_t numEnabled = countEnabledInstruments();
    if (numEnabled == 0) return NONE;
    
    m_currentInstrument = (m_currentInstrument + 1) % numEnabled;
    return getEnabledInstrument(m_currentInstrument);
}

uint8_t Distributor::nextInstrumentRoundRobinBalance() {
    // Find instrument with fewest active notes
    uint8_t bestInstrument = NONE;
    uint8_t minActiveNotes = 255;
    
    for (uint8_t i = 0; i < 32; i++) {
        if (distributorHasInstrument(i)) {
            uint8_t activeNotes = m_ptrInstrumentController->getNumActiveNotes(i);
            if (activeNotes < minActiveNotes) {
                minActiveNotes = activeNotes;
                bestInstrument = i;
            }
        }
    }
    
    return bestInstrument;
}

uint8_t Distributor::nextInstrumentSequential() {
    // Fill instruments in order
    for (uint8_t i = 0; i < 32; i++) {
        if (distributorHasInstrument(i)) {
            uint8_t activeNotes = m_ptrInstrumentController->getNumActiveNotes(i);
            if (activeNotes < m_numPolyphonicNotes) {
                return i;
            }
        }
    }
    
    return NONE;  // All instruments full
}

uint8_t Distributor::nextInstrumentRandom() {
    uint8_t numEnabled = countEnabledInstruments();
    if (numEnabled == 0) return NONE;
    
    uint8_t randomIndex = random(numEnabled);
    return getEnabledInstrument(randomIndex);
}

uint8_t Distributor::nextInstrumentStraightThrough() {
    // Map MIDI channel directly to instrument (if enabled)
    if (m_currentChannel < 32 && distributorHasInstrument(m_currentChannel)) {
        return m_currentChannel;
    }
    
    return NONE;
}

uint8_t Distributor::checkForNote(uint8_t note) {
    // Find which instrument is playing this note
    for (uint8_t i = 0; i < 32; i++) {
        if (distributorHasInstrument(i)) {
            if (m_ptrInstrumentController->isNoteActive(i, note)) {
                return i;
            }
        }
    }
    
    return NONE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

bool Distributor::distributorHasInstrument(uint8_t instrumentId) const {
    if (instrumentId >= 32) return false;
    return (m_instruments & (1UL << instrumentId)) != 0;
}

bool Distributor::channelEnabled(uint8_t channel) const {
    if (channel >= 16) return false;
    return (m_channels & (1 << channel)) != 0;
}

bool Distributor::noteInRange(uint8_t note) const {
    return note >= m_minNote && note <= m_maxNote;
}

uint8_t Distributor::countEnabledInstruments() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < 32; i++) {
        if (distributorHasInstrument(i)) {
            count++;
        }
    }
    return count;
}

uint8_t Distributor::getEnabledInstrument(uint8_t index) const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < 32; i++) {
        if (distributorHasInstrument(i)) {
            if (count == index) {
                return i;
            }
            count++;
        }
    }
    return NONE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

void Distributor::setDistributor(const uint8_t profile[DISTRIBUTOR_NUM_CFG_BYTES]) {
    fromSerial(profile);
}

void Distributor::setDistributionMethod(DistributionMethod method) {
    m_distributionMethod = method;
}

void Distributor::setMuted(bool muted) {
    m_muted = muted;
}

void Distributor::setDamperPedal(bool enable) {
    m_damperPedal = enable;
}

void Distributor::setPolyphonic(bool enable) {
    m_polyphonic = enable;
}

void Distributor::setNoteOverwrite(bool noteOverwrite) {
    m_noteOverwrite = noteOverwrite;
}

void Distributor::setMinMaxNote(uint8_t minNote, uint8_t maxNote) {
    m_minNote = minNote;
    m_maxNote = maxNote;
}

void Distributor::setNumPolyphonicNotes(uint8_t numPolyphonicNotes) {
    m_numPolyphonicNotes = numPolyphonicNotes;
}

void Distributor::setChannels(uint16_t channels) {
    m_channels = channels;
}

void Distributor::setInstruments(uint32_t instruments) {
    m_instruments = instruments;
}

void Distributor::toggleMuted() {
    m_muted = !m_muted;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Serialization
////////////////////////////////////////////////////////////////////////////////////////////////////

std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> Distributor::toSerial() const {
    std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> data{};
    
    // Boolean flags byte
    uint8_t distributorBoolByte = 0;
    if (m_muted) distributorBoolByte |= DISTRIBUTOR_BOOL_MUTED;
    if (m_damperPedal) distributorBoolByte |= DISTRIBUTOR_BOOL_DAMPERPEDAL;
    if (m_polyphonic) distributorBoolByte |= DISTRIBUTOR_BOOL_POLYPHONIC;
    if (m_noteOverwrite) distributorBoolByte |= DISTRIBUTOR_BOOL_NOTEOVERWRITE;
    
    // Match V1.0 format exactly (from table structure):
    // Cast Distributor Construct to uint8_t Array, every MSB = 0 as per MIDI Protocol
    data[0] = 0; // Distributor ID MSB (Generated in MsgHandler)
    data[1] = 0; // Distributor ID LSB (Generated in MsgHandler)
    data[2] = static_cast<uint8_t>((m_channels >> 14) & 0x03);  // Channels MSB bits (15->16)
    data[3] = static_cast<uint8_t>((m_channels >> 7) & 0x7F);   // Channels byte 1
    data[4] = static_cast<uint8_t>((m_channels >> 0) & 0x7F);   // Channels byte 2
    data[5] = static_cast<uint8_t>((m_instruments >> 28) & 0x0F); // Instruments MSB bits (29->32)
    data[6] = static_cast<uint8_t>((m_instruments >> 21) & 0x7F); // Instruments byte 1
    data[7] = static_cast<uint8_t>((m_instruments >> 14) & 0x7F); // Instruments byte 2
    data[8] = static_cast<uint8_t>((m_instruments >> 7) & 0x7F);  // Instruments byte 3
    data[9] = static_cast<uint8_t>((m_instruments >> 0) & 0x7F);  // Instruments byte 4
    data[10] = static_cast<uint8_t>(m_distributionMethod);        // Distribution method
    data[11] = distributorBoolByte;                               // Boolean values
    data[12] = m_minNote;                                         // Min note value
    data[13] = m_maxNote;                                         // Max note value
    data[14] = m_numPolyphonicNotes;                             // Number of polyphonic notes
    data[15] = 0; // Reserved
    
    // Bytes 16-23: Reserved for future use
    for (int i = 16; i < DISTRIBUTOR_NUM_CFG_BYTES; i++) {
        data[i] = 0;
    }
    
    return data;
}

void Distributor::fromSerial(const uint8_t data[DISTRIBUTOR_NUM_CFG_BYTES]) {
    // Deserialize distributor profile using the same layout as toSerial()
    // Layout (indices):
    // [0..1] = Distributor ID (MSB, LSB) - ignored here
    // [2] = channels MSB (2 bits)
    // [3] = channels byte 1 (7 bits)
    // [4] = channels byte 2 (7 bits)
    // [5]..[9] = instruments (5 * 7-bit segments, MSB first)
    // [10] = distribution method
    // [11] = boolean flags
    // [12] = min note
    // [13] = max note
    // [14] = num polyphonic notes

    // Channels: reconstruct 16-bit value from 3 bytes
    uint16_t ch_msb = static_cast<uint16_t>(data[2] & 0x03);
    uint16_t ch_mid = static_cast<uint16_t>(data[3] & 0x7F);
    uint16_t ch_lsb = static_cast<uint16_t>(data[4] & 0x7F);
    m_channels = (ch_msb << 14) | (ch_mid << 7) | ch_lsb;

    // Instruments: reconstruct 32-bit bitmask from 5 7-bit segments
    uint32_t inst_msb = static_cast<uint32_t>(data[5] & 0x0F);
    uint32_t inst_b1 = static_cast<uint32_t>(data[6] & 0x7F);
    uint32_t inst_b2 = static_cast<uint32_t>(data[7] & 0x7F);
    uint32_t inst_b3 = static_cast<uint32_t>(data[8] & 0x7F);
    uint32_t inst_b4 = static_cast<uint32_t>(data[9] & 0x7F);
    m_instruments = (inst_msb << 28) | (inst_b1 << 21) | (inst_b2 << 14) | (inst_b3 << 7) | inst_b4;

    // Boolean flags (stored at index 11)
    uint8_t boolByte = data[11];
    m_muted = (boolByte & DISTRIBUTOR_BOOL_MUTED) != 0;
    m_damperPedal = (boolByte & DISTRIBUTOR_BOOL_DAMPERPEDAL) != 0;
    m_polyphonic = (boolByte & DISTRIBUTOR_BOOL_POLYPHONIC) != 0;
    m_noteOverwrite = (boolByte & DISTRIBUTOR_BOOL_NOTEOVERWRITE) != 0;

    // Distribution method and note range
    m_distributionMethod = static_cast<DistributionMethod>(data[10]);
    m_minNote = data[12];
    m_maxNote = data[13];
    m_numPolyphonicNotes = data[14];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Diagnostics
////////////////////////////////////////////////////////////////////////////////////////////////////

// void Distributor::printStatus() const {
//     Serial.printf("=== %s Status ===\n", getName());
//     Serial.printf("Muted: %s\n", m_muted ? "Yes" : "No");
//     Serial.printf("Method: %s\n", 
//                   m_distributionMethod == DistributionMethod::RoundRobinBalance ? "RoundRobinBalance" :
//                   m_distributionMethod == DistributionMethod::RoundRobin ? "RoundRobin" :
//                   m_distributionMethod == DistributionMethod::Sequential ? "Sequential" :
//                   m_distributionMethod == DistributionMethod::Random ? "Random" :
//                   m_distributionMethod == DistributionMethod::StraightThrough ? "StraightThrough" : "Unknown");
    
//     // Print enabled channels
//     Serial.printf("Channels: ");
//     bool first = true;
//     for (uint8_t i = 0; i < 16; i++) {
//         if (channelEnabled(i)) {
//             if (!first) Serial.print(", ");
//             Serial.printf("%d", i + 1);  // Display as 1-16
//             first = false;
//         }
//     }
//     if (first) Serial.print("None");
//     Serial.println();
    
//     // Print enabled instruments
//     Serial.printf("Instruments: ");
//     first = true;
//     for (uint8_t i = 0; i < 32; i++) {
//         if (distributorHasInstrument(i)) {
//             if (!first) Serial.print(", ");
//             Serial.printf("%d", i);
//             first = false;
//         }
//     }
//     if (first) Serial.print("None");
//     Serial.println();
    
//     Serial.printf("Note Range: %d-%d\n", m_minNote, m_maxNote);
//     Serial.printf("Polyphonic Notes: %d\n", m_numPolyphonicNotes);
//     Serial.printf("Polyphonic: %s, Note Overwrite: %s, Damper Pedal: %s\n",
//                   m_polyphonic ? "Yes" : "No",
//                   m_noteOverwrite ? "Yes" : "No", 
//                   m_damperPedal ? "Yes" : "No");
// }