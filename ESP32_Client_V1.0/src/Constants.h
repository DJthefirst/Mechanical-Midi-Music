/*
 * Constants.h
 * Global constants for MIDI protocol, system configuration, and enumerations
 */

#pragma once
#include <array>
#include <cstdint>
using std::int8_t;
using std::int16_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
// General Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Bit masks
constexpr uint8_t MSB_BITMASK = 0x80;
constexpr uint8_t NONE = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////
// MIDI Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// MIDI message types
constexpr uint8_t MIDI_NoteOff            = 0x80;
constexpr uint8_t MIDI_NoteOn             = 0x90;
constexpr uint8_t MIDI_KeyPressure        = 0xA0;
constexpr uint8_t MIDI_ControlChange      = 0xB0;
constexpr uint8_t MIDI_ProgramChange      = 0xC0;
constexpr uint8_t MIDI_ChannelPressure    = 0xD0;
constexpr uint8_t MIDI_PitchBend          = 0xE0;
constexpr uint8_t MIDI_SysCommon          = 0xF0;

// System Common MIDI message types
constexpr uint8_t MIDI_SysEX    = 0x0;
constexpr uint8_t MIDI_SysEXEnd = 0x7;
constexpr uint8_t MIDI_SysStop  = 0xA;
constexpr uint8_t MIDI_SysReset = 0xF;

// Control Change controller types (handled across all active channels)
namespace MidiCC {
    constexpr uint8_t BankSelect = 0;
    constexpr uint8_t ModulationWheel = 1;
    constexpr uint8_t BreathControl = 2;
    constexpr uint8_t FootPedal = 4;
    constexpr uint8_t PortamentoTime = 5;
    constexpr uint8_t Volume = 7;
    constexpr uint8_t Pan = 10;
    constexpr uint8_t Expression = 11;
    constexpr uint8_t EffectCrtl_1 = 12;
    constexpr uint8_t EffectCrtl_2 = 13;
    constexpr uint8_t DamperPedal = 64;
    constexpr uint8_t Portamento = 65;
    constexpr uint8_t Sostenuto = 66;
    constexpr uint8_t SoftPedal = 67;
    constexpr uint8_t Legato = 68;
    constexpr uint8_t Hold2 = 69;

    // Channel Mode messages (handled across all active channels)
    constexpr uint8_t Mute = 120;
    constexpr uint8_t Reset = 121;
    constexpr uint8_t AllNotesOff = 123;
    constexpr uint8_t OmniModeOff = 124;
    constexpr uint8_t OmniModeOn = 125;
    constexpr uint8_t Monophonic = 126;
    constexpr uint8_t Polyphonic = 127;
};

// SysEx custom protocol

namespace SysEx {
    constexpr uint8_t ID = 0x7D; //Educational MIDI ID
    constexpr uint16_t Broadcast = 0x0000;
    constexpr uint16_t Server = 0x3FFF; //Server address is the MAX uint14

    constexpr uint8_t DeviceReady = 0x00;
    constexpr uint8_t ResetDeviceConfig = 0x01;
    constexpr uint8_t DiscoverDevices = 0x02;
    constexpr uint8_t Message = 0x03;
    constexpr uint8_t DeviceChanged = 0x04;

    constexpr uint8_t GetDeviceConstructWithDistributors = 0x10;
    constexpr uint8_t GetDeviceConstruct = 0x11;
    constexpr uint8_t GetDeviceID = 0x12;
    constexpr uint8_t GetDeviceName = 0x13;
    constexpr uint8_t GetDeviceBoolean = 0x14;

    constexpr uint8_t SetDeviceConstructWithDistributors = 0x20;
    constexpr uint8_t SetDeviceConstruct = 0x21;
    constexpr uint8_t SetDeviceID = 0x22;
    constexpr uint8_t SetDeviceName = 0x23;
    constexpr uint8_t SetDeviceBoolean = 0x24;

    constexpr uint8_t GetNumOfDistributors = 0x30;
    constexpr uint8_t AddDistributor = 0x31;
    constexpr uint8_t GetAllDistributors = 0x32;
    constexpr uint8_t RemoveDistributor = 0x33;
    constexpr uint8_t RemoveAllDistributors = 0x34;
    constexpr uint8_t ToggleMuteDistributor = 0x35;

    constexpr uint8_t GetDistributorConstruct = 0x40;
    constexpr uint8_t GetDistributorChannels = 0x41;
    constexpr uint8_t GetDistributorInstruments = 0x42;
    constexpr uint8_t GetDistributorMethod = 0x43;
    constexpr uint8_t GetDistributorBoolValues = 0x44;
    constexpr uint8_t GetDistributorMinMaxNotes = 0x45;
    constexpr uint8_t GetDistributorNumPolyphonicNotes = 0x46;

    constexpr uint8_t SetDistributor = 0x50;
    constexpr uint8_t SetDistributorChannels = 0x51;
    constexpr uint8_t SetDistributorInstruments = 0x52;
    constexpr uint8_t SetDistributorMethod = 0x53;
    constexpr uint8_t SetDistributorBoolValues = 0x54;
    constexpr uint8_t SetDistributorMinMaxNotes = 0x55;
    constexpr uint8_t SetDistributorNumPolyphonicNotes = 0x56;
};

//MIDI Constants
constexpr uint16_t MIDI_CTRL_CENTER = 0x2000;
constexpr uint8_t NUM_MIDI_CH = 16;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Instrument
{ 
    None = 0,
    PWM = 1,
    ShiftRegister,
    StepperMotor,
    FloppyDrive
};

enum class Platform
{ 
    _ESP32 = 1,
    _ESP8266,
    _ArduinoUno,
    _ArduinoMega,
    _ArduinoDue,
    _ArduinoMicro,
    _ArduinoNano
};

// Algorithmic methods to distribute notes amongst instruments
enum class DistributionMethod
{ 
    StraightThrough = 0,    // Each channel goes to the instrument with matching ID (e.g., Ch 10 -> Instrument 10)
    RoundRobin,             // Distributes notes in a circular manner
    RoundRobinBalance,      // Distributes notes in a circular manner (balances notes across instruments)
    Ascending,              // Plays note on lowest available instrument (balances notes across instruments)
    Descending,             // Plays note on highest available instrument (balances notes across instruments)
    Stack                   // TODO: Play notes polyphonically on lowest available instrument until full
};