/*
 * config.h - Default V2.0 Configuration
 *
 * This is the default configuration for V2.0 testing
 * Copy and modify this file for your specific hardware
 */

#pragma once

//=== Device Identity ===
#define DEVICE_NAME "MMM V2.0 Test Device"
#define DEVICE_ID 0x0042

//=== Hardware Configuration ===
#define PINS_LIST { 2, 4, 18, 19, 21, 22, 23, 32, 33, 25, 26, 27 }

//=== Component Selection ===
#include "Instruments/ESP32_PwmTimer.h"
using InstrumentType = ESP32_PwmTimer;

#include "Networks/SerialNetwork.h"
using NetworkType = SerialNetwork;

//=== Optional Settings ===
#define TIMER_RESOLUTION 1  // 1 microsecond for high precision
constexpr uint8_t MIN_NOTE_VALUE = 0;
constexpr uint8_t MAX_NOTE_VALUE = 127;

//=== Optional Features ===
#define ENABLE_LOCAL_STORAGE