/* 
 * Distributor.h
 *
 * Distributors distribute Midi Messages to instruments.
 *
 */

#pragma once

#include <stdint.h>
#include "Constants.h"
#include "Instruments/InstrumentController.h"

//Algorythmic Methods to Distribute Notes Amoungst Instruments.
enum DistributionMethod
{ 
    StraightThrough = 0, //Each Channel goes to the Instrument with a matching ID ex. Ch 10 -> Instrument 10
    RoundRobin,         //Distributes Notes in a circular manner.
    RoundRobinBalance,  //Distributes Notes in a circular manner (Balances Notes across Instruments).
    Accending,          //Plays Note on lowest available Instrument (Balances Notes across Instruments).
    Descending,         //Plays Note on highest available Instrument (Balances Notes across Instruments).
    Stack               //Play Notes Polyphonicaly on lowest available Instrument until full.
};

//Distributes Midi Messages to Instruments.
class Distributor{
private:

    //Local Atributes
    uint8_t m_currentChannel;
    uint8_t m_currentInstrument;
    InstrumentController* m_ptrInstrumentController;

    //Each Bit Represents an Enabled Channel/Instrument (limits max number of instruments to 32)
    uint16_t m_channels; //Represents Enabled MIDI Channels
    uint32_t m_instruments; //Represents Enabled Instruments

    //Settings
    bool m_damperPedal = false;
    bool m_polyphonic = true;
    bool m_noteOverwrite = false;
    uint8_t m_minNote, m_maxNote;
    uint8_t m_numPolyphonicNotes = 4;
    DistributionMethod m_distributionMethod = StraightThrough;

public:

    Distributor(InstrumentController* ptrInstrumentController);
    void ProcessMessage(uint8_t message[]);

    void GetDistributor(uint8_t profile[]);
    uint16_t GetChannels();

    void SetDistributor(uint8_t profile[]);
    void SetDistributionMethod(DistributionMethod);
    void SetDamperPedal(bool);
    void SetPolyphonic(bool);
    void SetNoteOverwrite(bool noteOverwrite);
    void SetMinMaxNote(uint8_t minNote, uint8_t maxNote);
    void SetNumPolyphonicNotes(uint8_t numPolyphonicNotes);
    void SetChannels(uint16_t channels);
    void SetInstruments(uint32_t instruments);

private:

    //Midi Message Events
    void NoteOnEvent(uint8_t key, uint8_t velocity);
    void NoteOffEvent(uint8_t key, uint8_t velocity);
    void KeyPressureEvent(uint8_t key, uint8_t velocity);
    void ControlChangeEvent(uint8_t controller, uint8_t value);
    void ProgramChangeEvent(uint8_t program);
    void ChannelPressureEvent( uint8_t velocity);
    void PitchBendEvent(uint16_t pitchBend);

    //Returns Instrument ID
    uint8_t NextInstrument();
    uint8_t CheckForNote(uint8_t note);
};