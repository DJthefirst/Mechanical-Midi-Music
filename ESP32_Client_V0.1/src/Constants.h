#pragma once
#include <stdint.h>

//Midi Msg Types
const uint8_t NoteOff = 0x80;
const uint8_t NoteOn = 0x90;
const uint8_t KeyPressure = 0xA0;
const uint8_t ControlChange = 0xB0;
const uint8_t ProgramChange = 0xC0;
const uint8_t ChannelPressure = 0xD0;
const uint8_t PitchBend = 0xE0;
const uint8_t SysCommon = 0xF0;

//SysCommon Midi Msg Types
const uint8_t SysEXE = 0x00;
const uint8_t SysStop = 0x0A;
const uint8_t SysReset = 0x0F;