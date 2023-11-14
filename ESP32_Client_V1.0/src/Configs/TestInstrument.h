#pragma once

#include "Instruments/InstrumentController.h"
#include "Instruments/PwmDriver.h"

//---------- Uncomment Your Selected Instrument Type ----------

//FloppyDrives  instrumentController; //Not Yet Implemented
PwmDriver     instrumentController;
//StepperL298n  instrumentController;
//StepperMotors instrumentController; //Not Yet Implemented
//ShiftRegister instrumentController;
//Dulcimer      instrumentController;

//---------- Define Your Device Settings ----------

    //Absolute max number of Polyphonic notes is 16
    #define MAX_POLYPHONIC_NOTES = 1
    //Absolute max number of Instruments is 32
    #define MAX_NUM_INSTRUMENTS = 8
    //Absolute Lowest Note Min=0
    #define MIN_MIDI_NOTE = 36 //C1 //36-83 dulcimer
    //Absolute Highest Note Max=127
    #define MAX_MIDI_NOTE = 83 //B4
    //A 14 bit number Representing this Devices ID
    #define SYSEX_DEV_ID = 0x01
    //Firmware Version 14bit
    #define FIRMWARE_VERSION = 03
    //Device Name
    #define DEVICE_NAME = "Test Instrument"
    //Interupt frequency. Faster produces accurate notes 
    // but leads to instability
    #define TIMER_RESOLUTION = 40

//---------- Uncomment Your Desired Settings ----------

    #define LOCAL_STORAGE_ENABLE

    #define LIGHTING = NONE