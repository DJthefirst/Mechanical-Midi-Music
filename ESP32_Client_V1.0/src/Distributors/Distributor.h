/* 
 * Distributor.h
 *
 * Distributors serve to route midi notes to varrious instrument groups
 * via configurable algorithms.Each Algorithm accounts for the number of 
 * active notes playing on each instrument and the order of instruments 
 * to be played.
 *  
 * ex. Midi ch 2 & 3 -> instruments 4-8 via RoundRobin.
 * 
 */

#pragma once

#include "../Device.h"
#include "../MsgHandling/MidiMessage.h"
#include "../Constants.h"
#include "DistributionStrategy.h"

// Forward declarations
class InstrumentControllerBase;

#include <array>
#include <cstdint>
#include <memory>
#include <bitset>
using std::int8_t;

//Size of Distributor when convered to Byte array
static const uint8_t DISTRIBUTOR_NUM_CFG_BYTES = 24;

static const uint8_t DISTRIBUTOR_BOOL_MUTED         = 0x01;
static const uint8_t DISTRIBUTOR_BOOL_DAMPERPEDAL   = 0x02;
static const uint8_t DISTRIBUTOR_BOOL_POLYPHONIC    = 0x04;
static const uint8_t DISTRIBUTOR_BOOL_NOTEOVERWRITE = 0x08;

/* Routes Midi Notes to various instrument groups via configurable algorithms. */
class Distributor{
private:

    std::shared_ptr<InstrumentControllerBase> m_instrumentController;

    //Each Bit Represents an Enabled Channel/Instrument (limits max number of instruments to 32)
    std::bitset<NUM_Channels> m_channels = 0; //Represents Enabled MIDI Channels
    std::bitset<NUM_Instruments> m_instruments = 0; //Represents Enabled Instruments

    //Strategy pattern for distribution methods
    std::unique_ptr<DistributionStrategy> m_distributionStrategy;

    //Settings
    bool m_muted = false;
    bool m_damperPedal = false;
    bool m_polyphonic = true;
    bool m_noteOverwrite = false;
    uint8_t m_minNote = 0;
    uint8_t m_maxNote = 127;
    uint8_t m_numPolyphonicNotes = 1;
    DistributionMethod m_distributionMethod = DistributionMethod::RoundRobinBalance;

public:

    friend class DistributionStrategy;

   Distributor();
    ~Distributor();

    // Disable copy (unique_ptr is not copyable) and enable move semantics
    Distributor(const Distributor&) = delete;
    Distributor& operator=(const Distributor&) = delete;
    Distributor(Distributor&&) noexcept = default;
    Distributor& operator=(Distributor&&) noexcept = default;

    /* Determines which instruments the message is for */
    void processMessage(MidiMessage message);

    /* Returns a Byte array representing this Distributor in 7-bit MIDI format */
    std::array<uint8_t,DISTRIBUTOR_NUM_CFG_BYTES> toSerial();

    bool getMuted() const;
    std::bitset<NUM_Channels> getChannels() const;
    std::bitset<NUM_Instruments> getInstruments() const;
    DistributionMethod getDistributionMethod() const;
    
    // Add routing method for messages
    void distributeMessage(MidiMessage& message);

    void setDistributor(uint8_t profile[]);
    void setDistributionMethod(DistributionMethod);
    void setMuted(bool muted);
    void setDamperPedal(bool enable);
    void setPolyphonic(bool enable);
    void setNoteOverwrite(bool noteOverwrite);
    void setMinMaxNote(uint8_t minNote, uint8_t maxNote);
    void setNumPolyphonicNotes(uint8_t numPolyphonicNotes);
    void setChannels(std::bitset<NUM_Channels> channels);
    void setInstruments(std::bitset<NUM_Instruments> instruments);

    void toggleMuted();

private:

    //Midi Message Events
    void noteOnEvent(uint8_t key, uint8_t velocity, uint8_t channel);
    void noteOffEvent(uint8_t key, uint8_t velocity, uint8_t channel);
    void keyPressureEvent(uint8_t key, uint8_t velocity);
    // void controlChangeEvent(uint8_t controller, uint8_t value);  --Implemented in MessageHandler--
    void programChangeEvent(uint8_t program);
    void channelPressureEvent( uint8_t velocity);
    void pitchBendEvent(uint16_t pitchBend, uint8_t channel);
    
    // Update the distribution strategy when method changes
    void updateDistributionStrategy();


    //-------- Helper Functions --------//

    /* Returns True if a Distributor Handles the given Instrument. */
    bool instrumentEnabled(int instrumentId) const noexcept;
    bool channelEnabled(uint8_t channel);
public:
    void stopActiveNotes();
};