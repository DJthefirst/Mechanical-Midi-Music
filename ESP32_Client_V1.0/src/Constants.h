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
const uint8_t SYSEX_ID = 0x7D; //Educational MIDI ID

const uint8_t SYSEX_DeviceReady = 0x00;
const uint8_t SYSEX_ResetDeviceConfig = 0x01;

const uint8_t SYSEX_GetDeviceConstructWithDistributors = 0x10;
const uint8_t SYSEX_GetDeviceConstruct = 0x11;
const uint8_t SYSEX_GetDeviceName = 0x12;
const uint8_t SYSEX_GetDeviceBoolean = 0x13;

const uint8_t SYSEX_SetDeviceConstructWithDistributors = 0x20;
const uint8_t SYSEX_SetDeviceConstruct = 0x21;
const uint8_t SYSEX_SetDeviceName = 0x22;
const uint8_t SYSEX_SetDeviceBoolean = 0x23;

const uint8_t SYSEX_RemoveDistributor = 0x30;
const uint8_t SYSEX_RemoveAllDistributors = 0x40;
const uint8_t SYSEX_GetNumOfDistributors = 0x31;
const uint8_t SYSEX_GetAllDistributors = 0x41;
const uint8_t SYSEX_AddDistributor = 0x32;
const uint8_t SYSEX_ToggleMuteDistributor = 0x42;

const uint8_t SYSEX_GetDistributorConstruct = 0x34;
const uint8_t SYSEX_GetDistributorChannels = 0x35;
const uint8_t SYSEX_GetDistributorInstruments = 0x36;
const uint8_t SYSEX_GetDistributorMethod = 0x37;
const uint8_t SYSEX_GetDistributorBoolValues = 0x38;
const uint8_t SYSEX_GetDistributorMinMaxNotes = 0x39;
const uint8_t SYSEX_GetDistributorNumPolyphonicNotes = 0x3A;

const uint8_t SYSEX_SetDistributor = 0x44;
const uint8_t SYSEX_SetDistributorChannels = 0x45;
const uint8_t SYSEX_SetDistributorInstruments = 0x46;
const uint8_t SYSEX_SetDistributorMethod = 0x47;
const uint8_t SYSEX_SetDistributorBoolValues = 0x48;
const uint8_t SYSEX_SetDistributorMinMaxNotes = 0x49;
const uint8_t SYSEX_SetDistributorNumPolyphonicNotes = 0x4A;

//MIDI Constants
const uint16_t MIDI_CTRL_CENTER = 0x2000;

////////////////////////////////////////////////////////////////////////////////////////////////////
//Enums
////////////////////////////////////////////////////////////////////////////////////////////////////

enum InstrumentType
{ 
    INSTRUMENT_PWM = 1,
    INSTRUMENT_ShiftRegister,
    INSTRUMENT_StepperMotor,
    INSTRUMENT_FloppyDrive
};

enum PlatformType
{ 
    PLATFORM_ESP32 = 1,
    PLATFORM_ESP8266,
    PLATFORM_ArduinoUno,
    PLATFORM_ArduinoMega,
    PLATFORM_ArduinoDue,
    PLATFORM_ArduinoMicro,
    PLATFORM_ArduinoNano
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