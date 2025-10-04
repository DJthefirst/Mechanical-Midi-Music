/*
 * config.h - V2.0 ESP32 PWM Configuration Example
 *
 * This is a V2.0 configuration equivalent to the ESP32_PWM.h from V1.0
 * Demonstrates the simplified 15-line configuration approach
 */

#pragma once

//=== Device Identity ===
#define DEVICE_NAME "ESP32 PWM V2"
#define DEVICE_ID 0x0042

//=== Hardware Configuration ===
#define PINS_LIST { 2, 4, 18, 19, 21, 22, 23, 32, 33, 25, 26, 27 }

//=== Component Selection ===
#include "instruments/ESP32_PwmTimer.h"
using InstrumentType = ESP32_PwmTimer;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;

//=== Optional Settings ===
#define TIMER_RESOLUTION 1  // 1 microsecond for high precision
#define MIN_NOTE 0
#define MAX_NOTE 127

//=== Optional Features ===
#define ENABLE_LOCAL_STORAGE
#define ENABLE_ADDRESSABLE_LEDS