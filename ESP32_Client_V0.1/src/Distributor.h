//
// Distributors distribute Midi Messages to instruments.
//
#pragma once

#include <stdint.h>
#include "Constants.h"
#include "InstrumentController.h"


//Algorythmic Methods to Distribute Notes Amoungst Instruments.
enum DistributionMethod
{ 
    StrightThrough = 0, //Each Channel goes to the Instrument with a matching ID ex. Ch 10 -> Instrument 10
    RoundRobin,         //Distributes Notes in a circular manner. 
    Accending,          //Plays Note on lowest available Instrument.
    Decending           //Plays Note on highest available Instrument.
};

class Distributor{
public:

    Distributor(InstrumentController* ptrInstrumentController);
    void processMessage(uint8_t message[]);

    void getDistributor(uint8_t profile[]);
    uint16_t getChannels();

    void setDistributor(uint8_t profile[]);
    void setNoteOverwrite(bool noteOverwrite);
    void setMinMaxNote(uint8_t minNote, uint8_t maxNote);
    void setNumPolyphonicNotes(uint8_t numPolyphonicNotes);
    void setChannels(uint16_t channels);
    void setInstruments(uint32_t instruments);

private:

    //Local Atributes
    uint8_t currentChannel;
    uint8_t currentInstrument;
    InstrumentController instrumentController;

    //Each Bit Represents an Enabled Channel/Instrument
    uint16_t _channels;
    uint32_t _instruments;

    //Settings
    bool _noteOverwrite = false;
    uint8_t _minNote, _maxNote;
    uint8_t _numPolyphonicNotes = 1;
    DistributionMethod _distributionMethod;

    //Midi Message Events
    void NoteOnEvent(uint8_t Key, uint8_t Velocity);
    void NoteOffEvent(uint8_t Key, uint8_t Velocity);
    void KeyPressureEvent(uint8_t Key, uint8_t Velocity);
    void ControlChangeEvent(uint8_t Controller, uint8_t Value);
    void ProgramChangeEvent(uint8_t Program);
    void ChannelPressureEvent( uint8_t Velocity);
    void PitchBendEvent(uint16_t PitchBend);

    //Returns Instrument ID
    uint8_t NextInstrument();
    uint8_t PreviousInstrument();
    uint8_t CheckForNote(uint8_t note);
};