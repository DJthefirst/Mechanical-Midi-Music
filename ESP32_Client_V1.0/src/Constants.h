/*
 * Constants.h
 *
 */

#pragma once
#include <array>
#include <cstdint>
using std::int8_t;
using std::int16_t;


//Bit Masks
const uint8_t MSB_BITMASK = 0x80;
const uint8_t NONE = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////
//MIDI Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

//Midi Msg Types
const uint8_t MIDI_NoteOff            = 0x80;
const uint8_t MIDI_NoteOn             = 0x90;
const uint8_t MIDI_KeyPressure        = 0xA0;
const uint8_t MIDI_ControlChange      = 0xB0;
const uint8_t MIDI_ProgramChange      = 0xC0;
const uint8_t MIDI_ChannelPressure    = 0xD0;
const uint8_t MIDI_PitchBend          = 0xE0;
const uint8_t MIDI_SysCommon          = 0xF0;

//SysCommon Midi Msg Types
const uint8_t MIDI_SysEX    = 0x0;
const uint8_t MIDI_SysEXEnd = 0x7;
const uint8_t MIDI_SysStop  = 0xA;
const uint8_t MIDI_SysReset = 0xF;

//CC Controller Types (Handled across all active channels)
const uint8_t MIDI_CC_BankSelect = 0;
const uint8_t MIDI_CC_ModulationWheel = 1;
const uint8_t MIDI_CC_BreathControl = 2;
const uint8_t MIDI_CC_FootPedal = 4;
const uint8_t MIDI_CC_PortamentoTime = 5;
const uint8_t MIDI_CC_Volume = 7;
const uint8_t MIDI_CC_Pan = 10;
const uint8_t MIDI_CC_Expression = 11;
const uint8_t MIDI_CC_EffectCrtl_1 = 12;
const uint8_t MIDI_CC_EffectCrtl_2 = 13;
const uint8_t MIDI_CC_DamperPedal = 64;
const uint8_t MIDI_CC_Portamento = 65;
const uint8_t MIDI_CC_Sostenuto = 66;
const uint8_t MIDI_CC_SoftPedal = 67;
const uint8_t MIDI_CC_Legato = 68;
const uint8_t MIDI_CC_Hold2 = 69;

//CC Channel Mode Messages(Handled across all active channels)
const uint8_t MIDI_CC_Mute = 120;
const uint8_t MIDI_CC_Reset = 121;
const uint8_t MIDI_CC_AllNotesOff = 123;
const uint8_t MIDI_CC_OmniModeOff = 124;
const uint8_t MIDI_CC_OmniModeOn = 125;
const uint8_t MIDI_CC_Monophonic = 126;
const uint8_t MIDI_CC_Polyphonic = 127;

//SYSEX Custom Protocal
const uint8_t SYSEX_ID = 0x7D;

const uint8_t SYSEX_ResetDeviceConfig = 0x00;
const uint8_t SYSEX_GetDeviceConstruct = 0x02;
const uint8_t SYSEX_GetDeviceName = 0x03;
const uint8_t SYSEX_GetDeviceBoolean = 0x04;
const uint8_t SYSEX_SetDeviceConstruct = 0x09;
const uint8_t SYSEX_SetDeviceName = 0x0A;
const uint8_t SYSEX_SetDeviceBoolean = 0x0B;

const uint8_t SYSEX_AddDistributor = 0x10;
const uint8_t SYSEX_RemoveDistributor = 0x11;
const uint8_t SYSEX_GetNumOfDistributors = 0x12;
const uint8_t SYSEX_GetDistributorConstruct = 0x14;
const uint8_t SYSEX_SetDistributorChannels = 0x15;
const uint8_t SYSEX_SetDistributorInstruments = 0x16;
const uint8_t SYSEX_SetDistributorMethod = 0x17;
const uint8_t SYSEX_SetDistributorBoolValues = 0x18;
const uint8_t SYSEX_SetDistributorMinMaxNotes = 0x19;
const uint8_t SYSEX_SetDistributorNumPolyphonicNotes = 0x1A;

//MIDI Constants
const uint16_t MIDI_CTRL_CENTER = 0x2000;

////////////////////////////////////////////////////////////////////////////////////////////////////
//Timer Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Number of octaves to bend notes by at full-deflection.
// Described as cents/cents-in-an-octave
constexpr float BEND_OCTAVES = 200/(float)1200;

/*
 * Number of microseconds in a timer-tick for setting timer resolution
 * and calculating noteTicks.  Smaller values here will trigger interrupts more often,
 * which might interfere with other processes but will result in more accurate frequency
 * reproduction.
 */
constexpr int TIMER_RESOLUTION = 40;

// The period of notes in microseconds
constexpr uint16_t notePeriods[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30578, 28861, 27242, 25713, 24270, 22909, 21622, 20409, 19263, 18182, 17161, 16198, //C1 - B1
    15289, 14436, 13621, 12856, 12135, 11454, 10811, 10205, 9632, 9091, 8581, 8099, //C2 - B2
    7645, 7218, 6811, 6428, 6068, 5727, 5406, 5103, 4816, 4546, 4291, 4050, //C3 - B3
    3823, 3609, 3406, 3214, 3034, 2864, 2703, 2552, 2408, 2273, 2146, 2025, //C4 - B4
	1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, //C5 - B5
	956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506, //C6 - B6
	478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, //C7 - B7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0
};

// Create Note Arrays in terms of Ticks(Interupt Intervals)
constexpr std::array<std::uint16_t,128> compute_divided_ticks(const int divisor)
{
    std::array<std::uint16_t,128> noteDoubleTicks{};
    for(int i=0;i<128;++i)
        noteDoubleTicks[i] = notePeriods[i]/divisor;
    
    return noteDoubleTicks;
}

constexpr auto noteTicks = compute_divided_ticks(TIMER_RESOLUTION);

// In some cases a pulse will only happen every-other tick (e.g. if the tick is
// toggling a pin on and off and pulses happen on rising signal) so to simplify
// the math multiply the RESOLUTION by 2 here.
constexpr auto noteDoubleTicks = compute_divided_ticks(TIMER_RESOLUTION*2);