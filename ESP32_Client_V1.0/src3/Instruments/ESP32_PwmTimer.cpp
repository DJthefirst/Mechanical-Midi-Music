/*
 * ESP32_PwmTimer.cpp - ESP32 PWM Timer Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART4_INSTRUMENTS.md specifications
 */

#ifdef ARDUINO_ARCH_ESP32

#include "ESP32_PwmTimer.h"

// Static member definitions
std::array<ESP32_PwmTimer::State, 12> ESP32_PwmTimer::s_states{};
std::array<uint16_t, 16> ESP32_PwmTimer::s_pitchBend{};

#endif  // ARDUINO_ARCH_ESP32