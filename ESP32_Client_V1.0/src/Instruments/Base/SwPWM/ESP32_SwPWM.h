#pragma once

#include "Constants.h"
#include "Config.h"
#include "Instruments/InstrumentControllerBase.h"
#include <cstdint>
using std::int8_t;

#ifndef INSTRUMENT_TYPE
    #define INSTRUMENT_TYPE ESP32_SwPWM
#endif

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class ESP32_SwPWM : public InstrumentControllerBase{
protected:
    static void Tick();
    static void togglePin(uint8_t instrument);

    //[Instrument][ActiveNote] MSB is set if note is Active the 7 LSBs are the Notes Value 
    static std::array<uint8_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_activeNotes;
    static uint8_t m_numActiveNotes;

    //Instrument Attributes
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_notePeriod;  //Base Note
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_activePeriod;//Note Played
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_currentTick; //Timeing
    static std::bitset<HardwareConfig::MAX_NUM_INSTRUMENTS> m_currentState; //IO

    //Vibrato Attributes
    static std::array<uint16_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_vibratoPhase; //Vibrato phase counter
    static std::array<uint8_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_vibratoDepth;   //Vibrato depth (controlled by modulation wheel)
    static std::array<uint8_t,HardwareConfig::MAX_NUM_INSTRUMENTS> m_vibratoRate;    //Vibrato rate (speed of oscillation)


public: 
    ESP32_SwPWM();
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t velocity) override;
    void stopAll() override;

    void setPitchBend(uint8_t channel, uint16_t value) override;
    void setModulationWheel(uint8_t channel, uint8_t value) override;

    Instrument getInstrumentType() const override { return Instrument::SW_PWM; }
    uint8_t getNumActiveNotes(uint8_t instrument) override;
    bool isNoteActive(uint8_t instrument, uint8_t note) override;

    //Timeout tracking functions
    void checkInstrumentTimeouts() override;

};