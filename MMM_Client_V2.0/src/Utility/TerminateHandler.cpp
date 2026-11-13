/*
 * TerminateHandler.cpp
 *
 * The firmware is built without exceptions, so an error raised inside the C++ standard
 * library (for example a failed allocation in std::vector) always ends in std::terminate().
 * GCC's default handler prints the demangled exception type first, which links the whole
 * C++ name demangler and printf (about 50 KB) into every Teensy build.
 *
 * Replacing it keeps the same outcome (report, then abort) without that code.
 * ESP32 builds are left alone: their images do not contain the demangler.
 */

#include "Config.h"

#if defined(PLATFORM_TEENSY41)

#include <Arduino.h>
#include <cstdlib>

namespace __gnu_cxx {
    void __verbose_terminate_handler() {
        Serial.println("Fatal C++ runtime error");
        std::abort();
    }
}

#endif // PLATFORM_TEENSY41
