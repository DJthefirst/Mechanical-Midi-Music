/*
 * SolenoidShiftRegCfg.h
 *
 * Configuration for solenoid instrument using two shift registers
 * First shift register: C3 (48) to B4 (71) - 24 solenoids
 * Second shift register: C5 (72) to B5 (83) - 12 solenoids
 * Total: 36 solenoids covering C3 to B5
 * 
 * Each solenoid activates for 200ms when a note is played
 */

#pragma once
#include <array>
#include <cstdint>

//---------- Device Configuration ----------
    #define DEVICE_NAME "Solenoid Shift Reg"
    #define DEVICE_ID 0x00020

    //Max Number of Instrument groups is 32
    #define NUM_INSTRUMENTS_VALUE 36
    //Multiplies Instrument groups into individual instruments
    #define NUM_SUBINSTRUMENTS_VALUE 1

    //Absolute Lowest Note (C3 = 48)
    #define MIN_NOTE_VALUE 48
    //Absolute Highest Note (B5 = 83)
    #define MAX_NOTE_VALUE 83

//---------- Hardware Configuration ----------
namespace HardwareConfig {

    #define INSTRUMENT_TIMEOUT_MS_VALUE 10000

    // Shift Register Component Configuration
    #define COMPONENT_SHIFTREG
        #define SHIFTREG_TYPE_VALUE HW_DEFAULT
        
        // First Shift Register (C3-B4, 24 outputs)
        #define SHIFTREG1_PIN_SER_VALUE 13    // Serial Data
        #define SHIFTREG1_PIN_CLK_VALUE 14    // Clock
        #define SHIFTREG1_PIN_LD_VALUE 15     // Latch/Load
        #define SHIFTREG1_PIN_EN_VALUE 16     // Enable (active low)
        #define SHIFTREG1_PIN_RST_VALUE 17    // Reset (active low)
        
        // Second Shift Register (C5-B5, 12 outputs)
        #define SHIFTREG2_PIN_SER_VALUE 25    // Serial Data
        #define SHIFTREG2_PIN_CLK_VALUE 26    // Clock
        #define SHIFTREG2_PIN_LD_VALUE 27     // Latch/Load
        #define SHIFTREG2_PIN_EN_VALUE 32     // Enable (active low)
        #define SHIFTREG2_PIN_RST_VALUE 33    // Reset (active low)

    #define PIN_LED_Data 12
}

//---------- Uncomment Your Selected Instrument Type ----------

    #define INSTRUMENT_TYPE_VALUE "Instruments/DJthefirst/SolenoidShiftReg.h"

//---------- Uncomment Your Selected COM Types ----------
 
    #define MMM_NETWORK_SERIAL
    //#define MMM_NETWORK_UDP
    //#define MMM_NETWORK_USB
    //#define MMM_NETWORK_DIN

    #ifdef MMM_NETWORK_SERIAL
        #define MMM_NETWORK_SERIAL_BAUD 115200 // Standard MIDI baud rate
    #endif

    #ifdef MMM_NETWORK_UDP
        #define MMM_NETWORK_UDP_AsyncWebServer_PORT 80 // Default AsyncWebServer port
        #define MMM_NETWORK_UDP_ADDRESS 224, 5, 6, 7 // Default local network IP
        #define MMM_NETWORK_UDP_OTA_PORT 8337 // Default UDP port for MIDI
    #endif

//---------- Uncomment Your Desired Extras ----------

    //Save Configuration On PowerOff
    #define EXTRA_LOCAL_STORAGE

    //Addressable LED Lighting effects (optional)
    #define EXTRA_ADDRESSABLE_LEDS

    //FAST LED Variables
    #ifdef EXTRA_ADDRESSABLE_LEDS
        #define LED_PIN     12
        #define NUM_LEDS    36
        #define BRIGHTNESS  255
        #define LED_TYPE    WS2812B
        #define COLOR_ORDER GRB
        #define UPDATES_PER_SECOND 100
    #endif
