/*
 * Constants.h - V2.0 Configuration Helper Macros and Constants
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART2_CONFIG.md specifications
 */

#pragma once
#include <cstddef>  // For size_t
#include <array>
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////////////////////////
// General Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Bit Masks
constexpr uint8_t MSB_BITMASK = 0x80;
constexpr uint8_t NONE = static_cast<uint8_t>(-1);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration Helper Macros
////////////////////////////////////////////////////////////////////////////////////////////////////

// String validation - prevents buffer overruns
#define VALIDATE_STRING_LENGTH(str, maxLen) \
    static_assert(sizeof(str) <= maxLen, "String exceeds maximum length of " #maxLen)

// Pin array size calculation helper
#define COUNT_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,N,...) N
#define COUNT_ARGS(...) COUNT_ARGS_IMPL(__VA_ARGS__,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

////////////////////////////////////////////////////////////////////////////////////////////////////
// MIDI Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// MIDI Message Types
constexpr uint8_t MIDI_NoteOff            = 0x80;
constexpr uint8_t MIDI_NoteOn             = 0x90;
constexpr uint8_t MIDI_KeyPressure        = 0xA0;
constexpr uint8_t MIDI_ControlChange      = 0xB0;
constexpr uint8_t MIDI_ProgramChange      = 0xC0;
constexpr uint8_t MIDI_ChannelPressure    = 0xD0;
constexpr uint8_t MIDI_PitchBend          = 0xE0;
constexpr uint8_t MIDI_SysCommon          = 0xF0;

// SysCommon MIDI Message Types
constexpr uint8_t MIDI_SysEX    = 0x0;
constexpr uint8_t MIDI_SysEXEnd = 0x7;
constexpr uint8_t MIDI_SysStop  = 0xA;
// Additional constants for V1.0 compatibility
constexpr uint16_t SYSEX_DEV_ID = 0x0001; // Default device ID  
constexpr uint16_t FIRMWARE_VERSION = 0x0200; // V2.0
constexpr uint8_t MIDI_SysReset = 0xF;

// CC Controller Types (Handled across all active channels)
constexpr uint8_t MIDI_CC_BankSelect = 0;
constexpr uint8_t MIDI_CC_ModulationWheel = 1;
constexpr uint8_t MIDI_CC_BreathControl = 2;
constexpr uint8_t MIDI_CC_FootPedal = 4;
constexpr uint8_t MIDI_CC_PortamentoTime = 5;
constexpr uint8_t MIDI_CC_Volume = 7;
constexpr uint8_t MIDI_CC_Pan = 10;
constexpr uint8_t MIDI_CC_Expression = 11;
constexpr uint8_t MIDI_CC_EffectCtrl_1 = 12;
constexpr uint8_t MIDI_CC_EffectCtrl_2 = 13;

// Additional CC types for completeness
constexpr uint8_t MIDI_CC_DamperPedal = 64;
constexpr uint8_t MIDI_CC_Portamento = 65;
constexpr uint8_t MIDI_CC_Sostenuto = 66;
constexpr uint8_t MIDI_CC_SoftPedal = 67;
constexpr uint8_t MIDI_CC_Legato = 68;
constexpr uint8_t MIDI_CC_Hold2 = 69;

// CC Channel Mode Messages (Handled across all active channels)
constexpr uint8_t MIDI_CC_Mute = 120;
constexpr uint8_t MIDI_CC_Reset = 121;
constexpr uint8_t MIDI_CC_AllNotesOff = 123;
constexpr uint8_t MIDI_CC_OmniModeOff = 124;
constexpr uint8_t MIDI_CC_OmniModeOn = 125;
constexpr uint8_t MIDI_CC_Monophonic = 126;
constexpr uint8_t MIDI_CC_Polyphonic = 127;

// MIDI Constants
constexpr uint16_t MIDI_CTRL_CENTER = 0x2000;
constexpr uint8_t NUM_MIDI_CH = 16;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Enums from V1.0 - Preserved for compatibility
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Instrument : uint8_t {
    FloppyDrive = 0,
    FloppyDriveBass = 1,
    FloppyDriveStacked = 2,
    PwmDriver = 3,
    PwmDriverStacked = 4,
    StepperA4988 = 5,
    StepperL298n = 6,
    ShiftRegister = 7,
    FloppySynth = 8,
    InstrAD9833 = 9,
    StepperSynth = 10,
    StepperSynthStacked = 11,
    Dulcimer = 12,
    DrumSimple = 13,
    Pwm8A04 = 14,
    PWM = 15,
    ESP32_PWM = 16,
    Custom = 255
};

enum class Platform : uint8_t {
    _ESP32 = 0,
    _ESP8266 = 1,
    ArduinoUno = 2,
    ArduinoMega = 3,
    ArduinoDue = 4,
    ArduinoMicro = 5,
    ArduinoNano = 6,
    Custom = 255
};

enum class DistributionMethod : uint8_t {
    RoundRobinBalance = 0,  // Balance active notes across instruments  
    RoundRobin = 1,         // Simple round-robin
    Sequential = 2,         // Fill instruments in order (was "Ascending")
    Random = 3,             // Random instrument selection
    StraightThrough = 4,    // 1:1 channel to instrument mapping
    Custom = 255            // User-defined algorithm
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SysEx Protocol Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Device identification
constexpr uint8_t SYSEX_ID = 0x7D;  // Educational MIDI ID (matches V1.0)
constexpr uint16_t SYSEX_Broadcast = 0x0000;
constexpr uint16_t SYSEX_Server = 0x3FFF; // Server address is the MAX uint14

// System Commands (0x00-0x0F)
constexpr uint8_t SYSEX_DeviceReady = 0x00;
constexpr uint8_t SYSEX_ResetDeviceConfig = 0x01;
constexpr uint8_t SYSEX_DiscoverDevices = 0x02;
constexpr uint8_t SYSEX_Message = 0x03;

// Get Device Commands (0x10-0x1F)
constexpr uint8_t SYSEX_GetDeviceConstructWithDistributors = 0x10;
constexpr uint8_t SYSEX_GetDeviceConstruct = 0x11;
constexpr uint8_t SYSEX_GetDeviceName = 0x12;
constexpr uint8_t SYSEX_GetDeviceBoolean = 0x13;

// Set Device Commands (0x20-0x2F)
constexpr uint8_t SYSEX_SetDeviceConstructWithDistributors = 0x20;
constexpr uint8_t SYSEX_SetDeviceConstruct = 0x21;
constexpr uint8_t SYSEX_SetDeviceName = 0x22;
constexpr uint8_t SYSEX_SetDeviceBoolean = 0x23;

// Distributor Commands (0x30-0x3F)
constexpr uint8_t SYSEX_GetNumDistributors = 0x30;
constexpr uint8_t SYSEX_AddDistributor = 0x31;
constexpr uint8_t SYSEX_GetAllDistributors = 0x32;
constexpr uint8_t SYSEX_RemoveDistributor = 0x33;
constexpr uint8_t SYSEX_RemoveAllDistributors = 0x34;
constexpr uint8_t SYSEX_ToggleMuteDistributor = 0x35;

// Get Distributor Commands (0x40-0x4F)
constexpr uint8_t SYSEX_GetDistributorConstruct = 0x40;
constexpr uint8_t SYSEX_GetDistributorChannels = 0x41;
constexpr uint8_t SYSEX_GetDistributorInstruments = 0x42;
constexpr uint8_t SYSEX_GetDistributorMethod = 0x43;
constexpr uint8_t SYSEX_GetDistributorBoolValues = 0x44;
constexpr uint8_t SYSEX_GetDistributorMinMaxNotes = 0x45;
constexpr uint8_t SYSEX_GetDistributorNumPolyphonicNotes = 0x46;

// Set Distributor Commands (0x50-0x5F)
constexpr uint8_t SYSEX_SetDistributorConstruct = 0x50;
constexpr uint8_t SYSEX_SetDistributorChannels = 0x51;
constexpr uint8_t SYSEX_SetDistributorInstruments = 0x52;
constexpr uint8_t SYSEX_SetDistributorMethod = 0x53;
constexpr uint8_t SYSEX_SetDistributorBoolValues = 0x54;
constexpr uint8_t SYSEX_SetDistributorMinMaxNotes = 0x55;
constexpr uint8_t SYSEX_SetDistributorNumPolyphonicNotes = 0x56;

// Legacy aliases for compatibility
constexpr uint8_t SYSEX_GetNumOfDistributors = SYSEX_GetNumDistributors;
constexpr uint8_t SYSEX_CMD_SET_DISTRIBUTOR = SYSEX_SetDistributorConstruct;
constexpr uint8_t SYSEX_CMD_GET_DISTRIBUTOR = SYSEX_GetDistributorConstruct;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Serialized distributor size (24 bytes)
constexpr uint8_t DISTRIBUTOR_NUM_CFG_BYTES = 24;

// Boolean flags for distributor settings
constexpr uint8_t DISTRIBUTOR_BOOL_MUTED         = 0x01;
constexpr uint8_t DISTRIBUTOR_BOOL_DAMPERPEDAL   = 0x02;
constexpr uint8_t DISTRIBUTOR_BOOL_POLYPHONIC    = 0x04;
constexpr uint8_t DISTRIBUTOR_BOOL_NOTEOVERWRITE = 0x08;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Configuration Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Device configuration storage
constexpr uint8_t DEVICE_NUM_NAME_BYTES = 20;
constexpr uint8_t DEVICE_NAME_OFFSET = 20;
constexpr uint8_t DEVICE_NUM_CFG_BYTES = 40;
constexpr uint8_t DEVICE_BOOL_OMNIMODE = 0x01;

// Maximum limits
constexpr uint8_t MAX_INSTRUMENTS = 32;  // Limited by 32-bit bitmask
constexpr uint8_t MAX_MIDI_CHANNELS = 16;
constexpr uint8_t MAX_MIDI_NOTE = 127;
constexpr uint8_t MIN_MIDI_NOTE_VAL = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Network Constants  
////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr uint8_t NETWORK_MIN_MSG_BYTES = 3;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Build Information
////////////////////////////////////////////////////////////////////////////////////////////////////

// Firmware version - can be overridden at build time
#ifndef BUILD_VERSION
    #define BUILD_VERSION 2
#endif

constexpr uint16_t DEFAULT_FIRMWARE_VERSION = BUILD_VERSION;