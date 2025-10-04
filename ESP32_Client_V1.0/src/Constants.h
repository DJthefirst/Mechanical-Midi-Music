/*
 * Constants.h
 *
 */

#pragma once
#include <array>
#include <cstdint>
using std::int8_t;
using std::int16_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
//General Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

//Bit Masks
constexpr uint8_t MSB_BITMASK = 0x80;
constexpr uint8_t NONE = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////
//MIDI Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

//Midi Msg Types
constexpr uint8_t MIDI_NoteOff            = 0x80;
constexpr uint8_t MIDI_NoteOn             = 0x90;
constexpr uint8_t MIDI_KeyPressure        = 0xA0;
constexpr uint8_t MIDI_ControlChange      = 0xB0;
constexpr uint8_t MIDI_ProgramChange      = 0xC0;
constexpr uint8_t MIDI_ChannelPressure    = 0xD0;
constexpr uint8_t MIDI_PitchBend          = 0xE0;
constexpr uint8_t MIDI_SysCommon          = 0xF0;

//SysCommon Midi Msg Types
constexpr uint8_t MIDI_SysEX    = 0x0;
constexpr uint8_t MIDI_SysEXEnd = 0x7;
constexpr uint8_t MIDI_SysStop  = 0xA;
constexpr uint8_t MIDI_SysReset = 0xF;

//CC Controller Types (Handled across all active channels)
constexpr uint8_t MIDI_CC_BankSelect = 0;
constexpr uint8_t MIDI_CC_ModulationWheel = 1;
constexpr uint8_t MIDI_CC_BreathControl = 2;
constexpr uint8_t MIDI_CC_FootPedal = 4;
constexpr uint8_t MIDI_CC_PortamentoTime = 5;
constexpr uint8_t MIDI_CC_Volume = 7;
constexpr uint8_t MIDI_CC_Pan = 10;
constexpr uint8_t MIDI_CC_Expression = 11;
constexpr uint8_t MIDI_CC_EffectCrtl_1 = 12;
constexpr uint8_t MIDI_CC_EffectCrtl_2 = 13;
constexpr uint8_t MIDI_CC_DamperPedal = 64;
constexpr uint8_t MIDI_CC_Portamento = 65;
constexpr uint8_t MIDI_CC_Sostenuto = 66;
constexpr uint8_t MIDI_CC_SoftPedal = 67;
constexpr uint8_t MIDI_CC_Legato = 68;
constexpr uint8_t MIDI_CC_Hold2 = 69;

//CC Channel Mode Messages(Handled across all active channels)
constexpr uint8_t MIDI_CC_Mute = 120;
constexpr uint8_t MIDI_CC_Reset = 121;
constexpr uint8_t MIDI_CC_AllNotesOff = 123;
constexpr uint8_t MIDI_CC_OmniModeOff = 124;
constexpr uint8_t MIDI_CC_OmniModeOn = 125;
constexpr uint8_t MIDI_CC_Monophonic = 126;
constexpr uint8_t MIDI_CC_Polyphonic = 127;

//SYSEX Custom Protocal
constexpr uint8_t SYSEX_ID = 0x7D; //Educational MIDI ID
constexpr uint16_t SYSEX_Broadcast = 0x0000;
constexpr uint16_t SYSEX_Server = 0x3FFF; //Server address is the MAX uint14

constexpr uint8_t SYSEX_DeviceReady = 0x00;
constexpr uint8_t SYSEX_ResetDeviceConfig = 0x01;
constexpr uint8_t SYSEX_DiscoverDevices = 0x02;
constexpr uint8_t SYSEX_Message = 0x03;

constexpr uint8_t SYSEX_GetDeviceConstructWithDistributors = 0x10;
constexpr uint8_t SYSEX_GetDeviceConstruct = 0x11;
constexpr uint8_t SYSEX_GetDeviceID = 0x12;
constexpr uint8_t SYSEX_GetDeviceName = 0x13;
constexpr uint8_t SYSEX_GetDeviceBoolean = 0x14;

constexpr uint8_t SYSEX_SetDeviceConstructWithDistributors = 0x20;
constexpr uint8_t SYSEX_SetDeviceConstruct = 0x21;
constexpr uint8_t SYSEX_SetDeviceID = 0x22;
constexpr uint8_t SYSEX_SetDeviceName = 0x23;
constexpr uint8_t SYSEX_SetDeviceBoolean = 0x24;

constexpr uint8_t SYSEX_GetNumOfDistributors = 0x30;
constexpr uint8_t SYSEX_AddDistributor = 0x31;
constexpr uint8_t SYSEX_GetAllDistributors = 0x32;
constexpr uint8_t SYSEX_RemoveDistributor = 0x33;
constexpr uint8_t SYSEX_RemoveAllDistributors = 0x34;
constexpr uint8_t SYSEX_ToggleMuteDistributor = 0x35;

constexpr uint8_t SYSEX_GetDistributorConstruct = 0x40;
constexpr uint8_t SYSEX_GetDistributorChannels = 0x41;
constexpr uint8_t SYSEX_GetDistributorInstruments = 0x42;
constexpr uint8_t SYSEX_GetDistributorMethod = 0x43;
constexpr uint8_t SYSEX_GetDistributorBoolValues = 0x44;
constexpr uint8_t SYSEX_GetDistributorMinMaxNotes = 0x45;
constexpr uint8_t SYSEX_GetDistributorNumPolyphonicNotes = 0x46;

constexpr uint8_t SYSEX_SetDistributor = 0x50;
constexpr uint8_t SYSEX_SetDistributorChannels = 0x51;
constexpr uint8_t SYSEX_SetDistributorInstruments = 0x52;
constexpr uint8_t SYSEX_SetDistributorMethod = 0x53;
constexpr uint8_t SYSEX_SetDistributorBoolValues = 0x54;
constexpr uint8_t SYSEX_SetDistributorMinMaxNotes = 0x55;
constexpr uint8_t SYSEX_SetDistributorNumPolyphonicNotes = 0x56;

//MIDI Constants
constexpr uint16_t MIDI_CTRL_CENTER = 0x2000;
constexpr uint8_t NUM_MIDI_CH = 16;

////////////////////////////////////////////////////////////////////////////////////////////////////
//Enums
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

//Algorythmic Methods to Distribute Notes Amoungst Instruments.
enum class DistributionMethod
{ 
    StraightThrough = 0,    //Each Channel goes to the Instrument with a matching ID ex. Ch 10 -> Instrument 10
    RoundRobin,             //Distributes Notes in a circular manner.
    RoundRobinBalance,      //Distributes Notes in a circular manner (Balances Notes across Instruments).
    Ascending,              //Plays Note on lowest available Instrument (Balances Notes across Instruments).
    Descending,             //Plays Note on highest available Instrument (Balances Notes across Instruments).
    Stack                   //TODO Play Notes Polyphonicaly on lowest available Instrument until full.
};