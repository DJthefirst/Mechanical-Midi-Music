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

#include <array>
#include <cstdint>
using std::int8_t;
using std::int16_t;
using std::int32_t;

#include "Constants.h"
#include "Instruments/InstrumentController.h"
#include "MidiMessage.h"
#include "Device.h"

//Size of Distributor when convered to Byte array
static const uint8_t NUM_DISTRIBUTOR_CFG_BYTES = 16;

/* Routes Midi Notes to various instrument groups via configurable algorithms. */
class Distributor{
private:

    //Local Atributes
    uint8_t m_currentChannel = 0;
    uint8_t m_currentInstrument = 0;
    InstrumentController* m_ptrInstrumentController;

    //Each Bit Represents an Enabled Channel/Instrument (limits max number of instruments to 32)
    uint16_t m_channels = 0; //Represents Enabled MIDI Channels
    uint32_t m_instruments = 0; //Represents Enabled Instruments

    //Settings
    bool m_damperPedal = false;
    bool m_polyphonic = true;
    bool m_noteOverwrite = false;
    uint8_t m_minNote = 0;
    uint8_t m_maxNote = 127;
    uint8_t m_numPolyphonicNotes = 1;
    DistributionMethod m_distributionMethod = DistributionMethod::StraightThrough;

public:

   explicit Distributor(InstrumentController* ptrInstrumentController);

    /* Determines which instruments the message is for */
    void processMessage(MidiMessage message);

    /* Returns a Byte array representing this Distributor */
    std::array<uint8_t,NUM_DISTRIBUTOR_CFG_BYTES> toSerial();

    uint16_t getChannels();
    uint32_t getInstruments();

    void setDistributor(uint8_t profile[]);
    void setDistributionMethod(DistributionMethod);
    void setDamperPedal(bool);
    void setPolyphonic(bool);
    void setNoteOverwrite(bool noteOverwrite);
    void setMinMaxNote(uint8_t minNote, uint8_t maxNote);
    void setNumPolyphonicNotes(uint8_t numPolyphonicNotes);
    void setChannels(uint16_t channels);
    void setInstruments(uint32_t instruments);

private:

    //Midi Message Events
    void noteOnEvent(uint8_t key, uint8_t velocity, uint8_t channel);
    void noteOffEvent(uint8_t key, uint8_t velocity);
    void keyPressureEvent(uint8_t key, uint8_t velocity);
    void controlChangeEvent(uint8_t controller, uint8_t value);
    void programChangeEvent(uint8_t program);
    void channelPressureEvent( uint8_t velocity);
    void pitchBendEvent(uint16_t pitchBend);

    // Returns the instument ID of the next instrument to be played.
    uint8_t nextInstrument();
    // Returns the instrument ID that is playing the given note else NONE(-1) if no instrument found.
    uint8_t checkForNote(uint8_t note);


    //-------- Helper Functions --------//

    /* Returns True if a Distributor Handles the given Instrument. */
    bool distributorHasInstrument(int instrumentId);
};