/*
 * InterruptTimer.h
 * Utility class for a high-precision timer that's relatively platform-agnostic
 */
#pragma once

#include "Device.h"
#include <cstdint>
using std::int8_t;


namespace InterruptTimer {
    void initialize(uint32_t microseconds, void (*isr)());
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//Timer Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

// Number of octaves to bend notes by at full-deflection.
// Described as cents/cents-in-an-octave
constexpr float BEND_OCTAVES = 200/(float)1200;



// The frequency of notes in hz
constexpr double noteFrequency[128] = {
    8.175799, 8.661957, 9.177024, 9.722718, 10.300861, 10.913383, 11.562325, 12.249857, 12.978272, 13.750000, 14.567617, 15.433853, //C-1 - B-1
    16.351598, 17.323914, 18.354048, 19.445436, 20.601722, 21.826765, 23.124651, 24.499715, 25.956544, 27.500000, 29.135235, 30.867706, //C0 - B0
    32.703196, 34.647829, 36.708096, 38.890873, 41.203445, 43.653530, 46.249303, 48.999429, 51.913087, 55.000000, 58.270470, 61.735413, //C1 - B1
    65.406391, 69.295658, 73.416192, 77.781746, 82.406889, 87.307058, 92.498606, 97.998859, 103.826174, 110.000000, 116.540940, 123.470825, //C2 - B2
    130.812783, 138.591315, 146.832384, 155.563492, 164.813778, 174.614116, 184.997211, 195.997718, 207.652349, 220.000000, 233.081881, 246.941651, //C3 - B3
    261.625565, 277.182631, 293.664768, 311.127000, 329.627557, 349.228231, 369.994423, 391.995436, 415.304698, 440.000000, 466.163762, 493.883301, //C4 - B4
    523.251131, 554.365262, 587.329536, 622.254000, 659.255114, 698.456463, 739.988845, 783.990872, 830.609395, 880.000000, 932.327523, 987.766603, //C5 - B5
    1046.502261, 1108.730524, 1174.659072, 1244.507935, 1318.510228, 1396.912926, 1479.977691, 1567.981744, 1661.218790, 1760.000000, 1864.655046, 1975.533205, //C6 - B6
    2093.004522, 2217.461048, 2349.318143, 2489.015870, 2637.020455, 2793.825851, 2959.955382, 3135.963488, 3322.437581, 3520.000000, 3729.310092, 3951.066410, //C7 - B7
    4186.009045, 4434.922096, 4698.636287, 4978.031740, 5274.040911, 5587.651703, 5919.910763, 6271.926976, 6644.875161, 7040.000000, 7458.620184, 7902.132820, //C8 - B8
    8372.018090, 8869.844191, 9397.272573, 9956.063479, 10548.081821, 11175.303406, 11839.821527, 12543.853951
};


// The frequency of notes in hz
//constexpr uint16_t noteFrequency[128]

// The period of notes in microseconds
constexpr uint16_t notePeriods[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30578, 28861, 27242, 25713, 24270, 22909, 21622, 20409, 19263, 18182, 17161, 16198, //C1 - B1
    15289, 14436, 13621, 12856, 12135, 11454, 10811, 10205, 9632, 9091, 8581, 8099, //C2 - B2
    7645, 7218, 6811, 6428, 6068, 5727, 5406, 5103, 4816, 4546, 4291, 4050, //C3 - B3
    3823, 3609, 3406, 3214, 3034, 2864, 2703, 2552, 2408, 2273, 2146, 2025, //C4 - B4
	1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, //C5 - B5
	956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506, //C6 - B6
	478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, //C7 - B7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * Number of microseconds in a timer-tick for setting timer resolution
 * and calculating noteTicks.  Smaller values here will trigger interrupts more often,
 * which might interfere with other processes but will result in more accurate frequency
 * reproduction.
 */
// constexpr int TIMER_RESOLUTION = 40; //40

// Create Note Arrays in terms of Ticks(Interupt Intervals)
constexpr std::array<std::uint16_t,128> compute_divided_ticks(const int divisor)
{
    std::array<std::uint16_t,128> noteDoubleTicks{};
    for(int i=0;i<128;++i)
        noteDoubleTicks[i] = notePeriods[i]/divisor;
    
    return noteDoubleTicks;
}

constexpr auto noteTicks = compute_divided_ticks(TIMER_RESOLUTION);

// In some cases a pulse will only happen every-other tick (e.g. if the tick is
// toggling a pin on and off and pulses happen on rising signal) so to simplify
// the math multiply the RESOLUTION by 2 here.
constexpr auto noteDoubleTicks = compute_divided_ticks(TIMER_RESOLUTION*2);