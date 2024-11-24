/*
 * Device.h
 * A Struct Representing this devices configuration
 */

#pragma once

#include "Arduino.h"
#include "Constants.h"
#include <array>
#include <stdint.h>
#include <cstdint>
using std::int8_t;

////////////////////////////////////////////////////////////////////////////////////////////////////
//Device Config
////////////////////////////////////////////////////////////////////////////////////////////////////

//---------- Uncomment Your Selected Device Config ----------

  #include "Configs/AirCompressor.h"
  //#include "Configs/Dulcimer.h"
  //#include "Configs/TestInstrument.h"
  //#include "Configs/FloppyDrives.h"
  //#include "Configs/StepperMotor.h"
  //#include "Configs/ExampleConfig.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//Device Defaults
////////////////////////////////////////////////////////////////////////////////////////////////////

//Device Construct Constants
constexpr uint8_t DEVICE_NUM_NAME_BYTES = 20;
constexpr uint8_t DEVICE_NUM_CFG_BYTES = 30;
constexpr uint8_t DEVICE_BOOL_OMNIMODE = 0x01;

namespace Device{
    //Golbal Device Default Attribues
    inline std::string Name = "New Device";
    inline bool OmniMode = false;

    //---------- Default Pinout and Platform ----------

    #ifndef PLATFORM_ESP32
        //Valid Pins ESP32 |2 4 12 13 16 17 18 19 21 22 23 25 26 27 32 33|
        //constexpr std::array<uint8_t,16> pins = {2, 4, 12, 13, 16, 17, 18, 19, 21, 22, 23,
        //                                    25, 26, 27, 32, 33};
        constexpr Platform platform = Platform::_ESP32;

    #elif PLATFORM_ESP8266
        //#define ARDUINO_ARCH_ESP32
        uint8_t pins[] = {};

        const PlatformType platform = PLATFORM_ESP8266;

    #elif PLATFORM_ARDUINO_UNO
        //#define ARDUINO_ARCH_AVR
        const uint8_t pins[] = {};

        const char platform[] = "Arduino Uno";
        const PlatformType platform = PLATFORM_ArduinoUno;

    #elif PLATFORM_ARDUINO_MEGA
        //#define ARDUINO_ARCH_AVR
        const uint8_t pins[] = {};

        const char platform[] = "Arduino Mega";
        const PlatformType platform = PLATFORM_ArduinoMega;

    #elif PLATFORM_ARDUINO_DUE
        //#define ARDUINO_ARCH_
        const uint8_t pins[] = {};

        const char platform[] = "Arduino Due";
        const PlatformType platform = PLATFORM_ArduinoDue;

    #elif PLATFORM_ARDUINO_MICRO
        //#define ARDUINO_ARCH_AVR
        const uint8_t pins[] = {};

        const char platform[] = "Arduino Micro";
        const PlatformType platform = PLATFORM_ArduinoMicro;

    #elif PLATFORM_ARDUINO_NANO
        //#define ARDUINO_ARCH_AVR
        const uint8_t pins[] = {};

        const char platform[] = "Arduino Nano";
        const PlatformType platform = PLATFORM_ArduinoNano;

    #endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//Device Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

    static uint8_t GetDeviceBoolean(){
        uint8_t deviceBoolByte = 0;
        if(Device::OmniMode) deviceBoolByte |= (1 << 0); //bit 0
        return deviceBoolByte;
    }

    static std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> GetDeviceConstruct(){
        std::array<std::uint8_t,DEVICE_NUM_CFG_BYTES> deviceObj;

        deviceObj[0] = static_cast<uint8_t>((SYSEX_DEV_ID >> 7) & 0x7F); //Device ID MSB
        deviceObj[1] = static_cast<uint8_t>((SYSEX_DEV_ID >> 0) & 0x7F); //Device ID LSB
        deviceObj[2] = GetDeviceBoolean();
        deviceObj[3] = MAX_NUM_INSTRUMENTS;
        deviceObj[4] = static_cast<uint8_t>(INSTRUMENT_TYPE);
        deviceObj[5] = static_cast<uint8_t>(PLATFORM_TYPE);
        deviceObj[6] = MIN_MIDI_NOTE;
        deviceObj[7] = MAX_MIDI_NOTE;
        deviceObj[8] = static_cast<uint8_t>((FIRMWARE_VERSION >> 7) & 0x7F);
        deviceObj[9] = static_cast<uint8_t>((FIRMWARE_VERSION >> 0) & 0x7F);

        for(uint8_t i = 0; i < 20; i++){
            if (Device::Name.size() >  i) deviceObj[10+i] = Device::Name[i];
            else deviceObj[10+i] = 0;
        }

        return deviceObj;
    }
};
