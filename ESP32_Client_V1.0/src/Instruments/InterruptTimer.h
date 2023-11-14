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