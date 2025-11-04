#pragma once

#include "Constants.h"
#include "Instruments/InstrumentControllerBase.h"
#include "Instruments/Base/SwPWM/ESP32_SwPWM.h"

#include <cstdint>
using std::int8_t;

/* Outputs a PWM signal at the Notes Frequency on each Digital IO Pin */
class ESP32_StepSwPWM : public ESP32_SwPWM{
public:
    static constexpr Instrument Type = Instrument::SW_PWM;
private:
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

    //Local MIDI Device Atributes
    uint8_t m_program = 0;
    uint8_t m_channelPressure = 0;
    uint16_t m_pitchBend[Midi::NUM_CH];


public: 
    ESP32_StepSwPWM();
    void reset(uint8_t instrument) override;
    void resetAll() override;
    void playNote(uint8_t instrument, uint8_t note, uint8_t velocity,  uint8_t channel) override;
    void stopNote(uint8_t instrument, uint8_t velocity) override;
    void stopAll() override;

    Instrument getInstrumentType() const override { return Instrument::StepSW_PWM; }
    
    //Timeout tracking functions
    void checkInstrumentTimeouts() override;
};