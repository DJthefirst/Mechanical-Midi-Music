//
// Distributors distribute Midi Messages to instruments.
//
#pragma once

#include <stdint.h>
#include "Constants.h"
#include "Instruments/InstrumentController.h"


//Algorythmic Methods to Distribute Notes Amoungst Instruments.
enum DistributionMethod
{ 
    StrightThrough = 0, //Each Channel goes to the Instrument with a matching ID ex. Ch 10 -> Instrument 10
    RoundRobin,         //Distributes Notes in a circular manner.
    RoundRobinBalance,  //Distributes Notes in a circular manner (Balances Notes across Instruments).
    Accending,          //Plays Note on lowest available Instrument (Balances Notes across Instruments).
    Descending,         //Plays Note on highest available Instrument (Balances Notes across Instruments).
    Stack               //Play Notes Polyphonicaly on lowest available Instrument until full.
};

class Distributor{
private:

    //Local Atributes
    uint8_t _currentChannel;
    uint8_t _currentInstrument;
    InstrumentController* _ptrInstrumentController;

    //Each Bit Represents an Enabled Channel/Instrument (limits max number of instruments to 32)
    uint16_t _channels; //Represents Enabled MIDI Channels
    uint32_t _instruments; //Represents Enabled Instruments

    //Settings
    bool _damperPedal = false;
    bool _polyphonic = true;
    bool _noteOverwrite = false;
    uint8_t _minNote, _maxNote;
    uint8_t _numPolyphonicNotes = 4;
    DistributionMethod _distributionMethod = StrightThrough;

public:

    Distributor(InstrumentController* ptrInstrumentController);
    void processMessage(uint8_t message[]);

    void getDistributor(uint8_t profile[]);
    uint16_t getChannels();

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
    void NoteOnEvent(uint8_t Key, uint8_t Velocity);
    void NoteOffEvent(uint8_t Key, uint8_t Velocity);
    void KeyPressureEvent(uint8_t Key, uint8_t Velocity);
    void ControlChangeEvent(uint8_t Controller, uint8_t Value);
    void ProgramChangeEvent(uint8_t Program);
    void ChannelPressureEvent( uint8_t Velocity);
    void PitchBendEvent(uint16_t PitchBend);

    //Returns Instrument ID
    uint8_t NextInstrument();
    uint8_t CheckForNote(uint8_t note);
};