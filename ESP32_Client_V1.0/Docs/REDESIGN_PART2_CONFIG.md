# Part 2: Configuration System Design

## Overview

The configuration system is the user's primary interface to the MMM system. V1.0 requires users to edit multiple files with scattered settings. The pure template V2.0 made it worse (70+ lines). This improved design achieves a **10-15 line configuration** while adding compile-time validation.

## Current V1.0 Problems

### Example: Current FloppyDrives.h (40+ lines)
```cpp
#pragma once
#include <array>

// Scattered constants without clear organization
constexpr Instrument INSTRUMENT_TYPE = Instrument::FloppyDrive;
constexpr Platform PLATFORM_TYPE = Platform::_ESP32;
constexpr uint8_t MAX_POLYPHONIC_NOTES = 1;
constexpr uint8_t NUM_INSTRUMENTS = 4; 
constexpr uint8_t NUM_SUBINSTRUMENTS = 1; 
constexpr uint8_t MAX_NUM_INSTRUMENTS = NUM_INSTRUMENTS * NUM_SUBINSTRUMENTS; 
constexpr uint8_t MIN_MIDI_NOTE = 36; 
constexpr uint8_t MAX_MIDI_NOTE = 83;
constexpr uint16_t SYSEX_DEV_ID = 0x001;
constexpr uint16_t FIRMWARE_VERSION = 04;
constexpr std::array<uint8_t,10> pins = {26,27,2,4,18,19,21,22,23,32};
constexpr int TIMER_RESOLUTION = 8;

// ...then elsewhere in Device.h:
#if PLATFORM_TYPE == Platform::_ESP32
    #include "Instruments/Default/FloppyDrive.h"
    using instrumentType = FloppyDrive;
#endif

// ...and in main.cpp:
#include "Configs/FloppyDrives.h"
#include "Networks/NetworkSerial.h"
using networkType = NetworkSerial;
```

**Problems:**
- Settings scattered across 3+ files
- No clear hierarchy or grouping
- Magic numbers without explanation
- Requires understanding C++ syntax
- No validation until runtime
- Redundant settings (NUM_INSTRUMENTS vs pins.size())

## Improved V2.0 Configuration Design

### Goal: Single File, 10-15 Lines, Self-Documenting

```cpp
// config.h - Complete device configuration
#pragma once

//=== Device Identity ===
#define DEVICE_NAME "Floppy Orchestra"
#define DEVICE_ID 0x0042

//=== Hardware ===
#define PINS 26,27,2,4,18,19,21,22,23,32

### How It Works

This section clarifies practical choices for the config syntax and shows how to:
- switch config files at build-time (and optionally runtime),
- declare pins without fragile variadic-macro hacks, 
- pick networks using a clean templated `MultiNetwork<>` type,
- and configure per-instrument LED effects and mappings.

#### 1. Device Identity (Required)
```cpp
#define DEVICE_NAME "My Custom Device"  // Human-readable name (max 32 chars)
#define DEVICE_ID 0x0042                // Unique ID (1-16383)
```

DEVICE_NAME is used for BLE advertising and diagnostics. DEVICE_ID must be a 14-bit MMM device id (0x0001..0x3FFF).

#### 2. Hardware Configuration (Recommended)

Avoid fragile variadic macro counting. The simplest, clearest approach is to define a BRACED initializer for pins. This is explicit, works with the language (no preprocessor tricks), and is easy to edit.

```cpp
// config.h
#define DEVICE_NAME "Floppy Orchestra"
#define DEVICE_ID 0x0042

// Provide pins as a braced initializer - no COUNT_ARGS required
#define PINS_LIST { 26, 27, 2, 4, 18, 19, 21, 22, 23, 32 }
```

Device.h then converts that macro into a constexpr array:

```cpp
// Device.h
// Convert the braced initializer into a constexpr array. This trick uses a temporary
// initializer expression to let the compiler determine the length at compile time.
constexpr std::array<uint8_t, sizeof((int[]){PINS_LIST})/sizeof(int)> INSTRUMENT_PINS = PINS_LIST;
constexpr uint8_t NUM_INSTRUMENTS = static_cast<uint8_t>(INSTRUMENT_PINS.size());
```

Notes:
- This approach avoids COUNT_ARGS and is robust across compilers.
- If you prefer a single-line comma list, you can keep the older `PINS 26,27,...` style, but the braced `PINS_LIST` is recommended.

#### 3. Component Selection (Required)

Pick instrument and network types using includes + type aliases. For multiple networks we recommend using a templated `MultiNetwork` type which is both clearer and type-safe:

```cpp
#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

#include "networks/MultiNetwork.h"
#include "networks/SerialNetwork.h"
#include "networks/UdpNetwork.h"

// Clean, explicit: MultiNetwork<Primary, Secondary, ...>
using NetworkType = MultiNetwork<SerialNetwork, UdpNetwork>;
```

Device.h then aliases the runtime controller to this type:

```cpp
using NetworkController = NetworkType;
```

This replaces the older `SecondaryNetwork` macro and makes multiple-network configurations explicit.

#### 4. Optional Features (LEDs, Timer, Notes)

Optional settings are still macros for simplicity, but LED mapping is best expressed as a constexpr table rather than a single macro. See the "LED per-instrument mapping" sub-section below.

Example optional settings:

```cpp
#define ENABLE_LEDS
#define TIMER_RESOLUTION 8  // microseconds (default: 40)
#define MIN_NOTE 36
#define MAX_NOTE 83

#### Config file switching (build-time and runtime)

Build-time switching (recommended): use PlatformIO build flags to pick which `config.h` to include at compile time. Add an entry to `platformio.ini` like this:

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
build_flags = -DCONFIG_PATH=\"configs/my_floppy_config.h\"
```

Then in your top-level `Device.h` or main include file:

```cpp
#ifdef CONFIG_PATH
#  include CONFIG_PATH
#else
#  include "config.h"
#endif
```

Notes:
- The build flag sets `CONFIG_PATH` to a string literal usable by `#include`.
- You can create multiple env sections in `platformio.ini` for different configs (e.g., `env:stage`, `env:home`).

Runtime switching (optional): support loading an override from NVS/SPIFFS/SD at boot. Keep a compiled default `config.h` for safety, and implement a small runtime JSON loader that overrides only the runtime mappings (LED maps, distributor profiles, network server IP, etc.). This provides field reconfiguration without reflashing.

Example runtime override strategy (high-level):

1. On boot, call `LocalStorage::loadRuntimeConfig()`.
2. If a valid JSON is found, parse LED mappings, distributor blobs, and network endpoints and apply them.
3. If no valid runtime file found, fall back to compiled `config.h`.

This is optional but useful for live reconfiguration.

#### Notes on quoting and Windows PowerShell

When using PlatformIO on Windows PowerShell, escape quotes appropriately. Example `powershell` line to build using an alternate config:

```powershell
pio run -e esp32 -DCONFIG_PATH=\"configs\my_floppy_config.h\"
```
(PlatformIO's ini `build_flags` approach is usually easier than passing via CLI.)

### LED per-instrument mapping and effects

Goal: Allow mapping instrument IDs to LED ranges with per-event effects (note-on pulse, sustained hold, note-off fade, etc.). Provide both compile-time defaults (constexpr table) and runtime-editable mappings (saved to NVS).

Data model (recommended):

```cpp
// LedEffects.h
enum class LedEffect : uint8_t {
    None = 0,
    On,
    Pulse,
    Strobe,
    Fade,
    ColorCycle
};

struct LedMapping {
    uint8_t instrumentId;   // instrument index (0..31)
    uint16_t ledStart;      // first LED index (0-based)
    uint16_t ledCount;      // number of LEDs controlled
    LedEffect effect;       // default effect for NoteOn
    uint8_t colorPalette;   // optional palette index
};

// Compile-time default mapping (override in config.h or load at runtime)
constexpr std::array<LedMapping, 3> DEFAULT_LED_MAPPINGS = {
    LedMapping{ 0,  3, 6, LedEffect::Pulse, 0 }, // Inst 0 -> leds 3-8 pulse
    LedMapping{ 1,  9, 3, LedEffect::On,    1 }, // Inst 1 -> leds 9-11 steady
    LedMapping{ 2, 12, 4, LedEffect::Fade,  2 }  // Inst 2 -> leds 12-15 fade
};

// Expanded example: realistic mapping for a 16-LED strip and 8 instruments
constexpr std::array<LedMapping, 8> EXAMPLE_LED_MAPPINGS = {
    LedMapping{ 0,  0, 2, LedEffect::Pulse,     0 }, // 0: leds 0-1
    LedMapping{ 1,  2, 2, LedEffect::On,        1 }, // 1: leds 2-3
    LedMapping{ 2,  4, 2, LedEffect::Pulse,     0 }, // 2: leds 4-5
    LedMapping{ 3,  6, 2, LedEffect::Fade,      2 }, // 3: leds 6-7
    LedMapping{ 4,  8, 2, LedEffect::ColorCycle,3 }, // 4: leds 8-9
    LedMapping{ 5, 10, 2, LedEffect::Strobe,    4 }, // 5: leds 10-11
    LedMapping{ 6, 12, 2, LedEffect::Pulse,     0 }, // 6: leds 12-13
    LedMapping{ 7, 14, 2, LedEffect::Fade,      2 }  // 7: leds 14-15
};

// Example runtime JSON file (stored on SPIFFS/NVS or provided via network) to override mappings
// File: /config/led_mappings.json
// {
//   "mappings": [
//     { "instrumentId": 0, "ledStart": 0, "ledCount": 2, "effect": "Pulse", "palette": 0 },
//     { "instrumentId": 1, "ledStart": 2, "ledCount": 2, "effect": "On", "palette": 1 },
//     { "instrumentId": 2, "ledStart": 4, "ledCount": 4, "effect": "Fade", "palette": 2 }
//   ]
// }

```

Addressable LEDs API (integration points):

```cpp
// extras/AddressableLeds.h (interface sketch)
class AddressableLeds {
public:
    void begin();
    void setMappings(const std::vector<LedMapping>& mappings);

    // Called by MessageHandler when a note is played/stopped
    void onNoteOn(uint8_t instrumentId, uint8_t note, uint8_t velocity);
    void onNoteOff(uint8_t instrumentId, uint8_t note);

    // Called from main loop or a timer to update active effects
    void loop();
};
```

MessageHandler hooks (where to call the LED methods):

```cpp
// Inside Distributor::noteOnEvent (or MessageHandler after distribute)
if (hasLedController) {
    ledController.onNoteOn(instrumentId, Note, Velocity);
}

// Inside Distributor::noteOffEvent
if (hasLedController) {
    ledController.onNoteOff(instrumentId, Note);
}
```

Runtime editable mappings & NVS storage

Provide helper methods in `LocalStorage` to save/load LED mappings as a blob:

```cpp
// LocalStorage.h additions
static bool saveLedMappings(const std::vector<LedMapping>& mappings) {
    // nvs_set_blob("led_maps", data, len)
}

static bool loadLedMappings(std::vector<LedMapping>& out) {
    // nvs_get_blob("led_maps", buffer, &len)
}
```

At boot, prefer runtime-loaded mappings (if present) and fall back to `DEFAULT_LED_MAPPINGS` or a mapping provided in `config.h`.

Example `config.h` LED mapping override (compile-time):

```cpp
// config.h
#define ENABLE_LEDS
constexpr std::array<LedMapping, 2> LED_MAPPINGS = {
    LedMapping{0, 3, 6, LedEffect::Pulse, 0},
    LedMapping{1, 9, 3, LedEffect::On,    1}
};
```

MessageHandler/Device initialization pseudocode:

```cpp
void setup() {
    // Init hardware
    network.begin();
    instrument.resetAll();

    // LED setup
    #ifdef ENABLE_LEDS
    ledController.begin();
    std::vector<LedMapping> maps;
    if (LocalStorage::loadLedMappings(maps)) {
        ledController.setMappings(maps);
    } else {
        // fallback to compile-time mapping if present
        ledController.setMappings(std::vector<LedMapping>(LED_MAPPINGS.begin(), LED_MAPPINGS.end()));
    }
    #endif
}
```

Performance and memory notes

- Keep LED effect processing in a lightweight loop (no heavy allocations).
- Store mappings compactly in NVS (binary blob). Each mapping is small (8-12 bytes); 10 mappings = ~120 bytes.
- Avoid per-note dynamic allocations in the hot path; pre-allocate effect state arrays sized to total LED segments.

----

```

//=== Components ===
#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;

// Done! 15 lines total, everything configured.
```

### How It Works

#### 1. Device Identity (Required)
```cpp
#define DEVICE_NAME "My Custom Device"  // Human-readable name (max 32 chars)
#define DEVICE_ID 0x0042                // Unique ID (1-16383)
### Helper Macros / Small Utilities

We intentionally avoid brittle COUNT_ARGS macros. The recommended primitives are small extractors you can use only for parsing simple macros (e.g., ENABLE_LEDS) and a string-length validator:

```cpp
// Constants.h

// Note: small extractor macros (GET_ARG*) intentionally removed from the
// recommended primitives. Prefer explicit small macros or runtime parsing for
// ENABLE_LEDS and similar parameters to keep config parsing simple and robust.

// String validation
#define VALIDATE_STRING_LENGTH(str, maxLen) \
    static_assert(sizeof(str) <= maxLen, "String exceeds maximum length of " #maxLen)
```

If you must parse arbitrary-length variadic lists, prefer using a braced initializer (`PINS_LIST`) and let the compiler deduce the size from the initializer as shown above.
```

- **DEVICE_NAME**: Used in BLE advertising, debug output, device identification
- **DEVICE_ID**: 14-bit unique identifier for MMM protocol (0x0001-0x3FFF)
- **Auto-generated**: Firmware version from build system

#### 2. Hardware Configuration (Required)
```cpp
#define PINS 26,27,2,4,18,19,21,22,23,32
```

- **Simple comma-separated list** - no array syntax required
- **Expanded internally** to `constexpr std::array<uint8_t, N> INSTRUMENT_PINS = {PINS};`
- **Auto-detects count** - no manual NUM_INSTRUMENTS setting
- **Validated** at compile-time for pin conflicts

#### 3. Component Selection (Required)
```cpp
#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;
```

- **Type alias pattern** - familiar to V1.0 users
- **Include + using** - clear two-step process
- **Path guides users** - autocomplete works
- **Platform auto-detected** - Arduino vs ESP32 from build flags

#### 4. Optional Features (Optional)
```cpp
// Optional: Add LED support
#define ENABLE_LEDS 13, WS2812, 16  // Pin, Type, Count

// Optional: Add multiple networks
#include "networks/UdpNetwork.h"
using SecondaryNetwork = UdpNetwork;

// Optional: Custom settings
#define TIMER_RESOLUTION 8  // microseconds (default: 40)
#define MIN_NOTE 36         // MIDI note (default: 0)
#define MAX_NOTE 83         // MIDI note (default: 127)
```

## Configuration Validation System

### Compile-Time Checks

The existing validation ideas remain valid. A couple small changes:

- Use `INSTRUMENT_PINS` derived from `PINS_LIST` so the duplicate-pin check is straightforward.
- Keep platform-specific rules (ESP32 invalid pins) and name length checks.

```cpp
// DeviceValidation.h
namespace ConfigValidation {
    template<size_t N>
    constexpr bool hasDuplicatePins(const std::array<uint8_t, N>& pins) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = i + 1; j < N; ++j) {
                if (pins[i] == pins[j]) return true;
            }
        }
        return false;
    }

    static_assert(!hasDuplicatePins(INSTRUMENT_PINS),
        "Error: Duplicate pins detected in PINS_LIST");

    #ifdef PLATFORM_ESP32
    static_assert(pinsValidForESP32(INSTRUMENT_PINS),
        "Error: Invalid pins for ESP32 (34-39 are input-only, max pin 39)");
    #endif

    static_assert(sizeof(DEVICE_NAME) <= 32, "Error: DEVICE_NAME exceeds 32 characters");
    static_assert(INSTRUMENT_PINS.size() <= 32, "Error: Max 32 instruments supported");
}
```
(Implementation of `pinsValidForESP32` omitted above for brevity — keep the earlier logic.)
## Implementation Details

### Device.h - Configuration Processing

```cpp
// Device.h - Processes user configuration
#pragma once
#### 3. Complex Multi-Network Setup

#include "networks/UdpNetwork.h"

#define ENABLE_LEDS

// Use the templated MultiNetwork type in your config:
using NetworkType = MultiNetwork<SerialNetwork, UdpNetwork>;

#define LED_PIN 25
#define LED_TYPE WS2812
#define LED_COUNT 16

// 1. Include user configuration
#include "config.h"

// 2. Generate derived constants
namespace Config {
    // Device.h prefers PINS_LIST in configs. It converts that braced initializer
    // into a constexpr std::array usable throughout the codebase. If a legacy
    // comma-list `PINS` is present, Device.h wraps it with braces to achieve
    // the same result.
    constexpr std::array<uint8_t, sizeof((int[]){PINS_LIST})/sizeof(int)> INSTRUMENT_PINS = PINS_LIST;
    constexpr uint8_t NUM_INSTRUMENTS = static_cast<uint8_t>(INSTRUMENT_PINS.size());

    // Device identity
    constexpr const char* DEVICE_NAME_STR = DEVICE_NAME;
    constexpr uint16_t DEVICE_ID_VAL = DEVICE_ID;
    constexpr uint16_t FIRMWARE_VERSION = BUILD_VERSION;  // From build system
    
    // Optional settings with defaults
    #ifndef TIMER_RESOLUTION
        constexpr int TIMER_RESOLUTION = 40;
    #endif
    
    #ifndef MIN_NOTE
        constexpr uint8_t MIN_NOTE = 0;
    #endif
    
    #ifndef MAX_NOTE
        constexpr uint8_t MAX_NOTE = 127;
    #endif
}

// 3. Compile-time validation
static_assert(Config::NUM_INSTRUMENTS > 0, 
    "Error: No instrument pins defined. Add PINS to config.h");
static_assert(Config::DEVICE_ID_VAL > 0 && Config::DEVICE_ID_VAL < 0x4000,
    "Error: DEVICE_ID must be between 1 and 16383");
static_assert(Config::MIN_NOTE < Config::MAX_NOTE,
    "Error: MIN_NOTE must be less than MAX_NOTE");

// 4. Platform detection
#if defined(ARDUINO_ARCH_ESP32)
    #define PLATFORM_ESP32
    constexpr Platform PLATFORM_TYPE = Platform::ESP32;
#elif defined(ARDUINO_AVR_UNO)
    #define PLATFORM_ARDUINO_UNO
    constexpr Platform PLATFORM_TYPE = Platform::ArduinoUno;
#else
    #error "Unsupported platform. Add platform detection to Device.h"
#endif

// 5. Network configuration
#ifndef SecondaryNetwork
    using NetworkController = NetworkType;  // Single network
#else
    using NetworkController = MultiNetwork<NetworkType, SecondaryNetwork>;
#endif

// 6. LED configuration (optional)
#ifdef ENABLE_LEDS
    #include "extras/AddressableLeds.h"
    // Prefer runtime construction: provide LED params in macros or runtime JSON
    // and construct AddressableLeds(pin, type, count) in setup(). This avoids
    // complex preprocessor parsing and improves readability.
    // Example runtime usage shown in Device initialization pseudocode below.
    #define HAS_LEDS
#endif
```

### Helper Macros

```cpp
// Constants.h - Configuration helper macros

// Small extractor macros (use sparingly and only for fixed-format macros)
#define GET_ARG1(a, ...) a
#define GET_ARG2(a, b, ...) b
#define GET_ARG3(a, b, c, ...) c

// String validation
#define VALIDATE_STRING_LENGTH(str, maxLen) \
    static_assert(sizeof(str) <= maxLen, "String exceeds maximum length of " #maxLen)
```

## Configuration Validation System

### Compile-Time Checks

```cpp
// DeviceValidation.h - Comprehensive validation

namespace ConfigValidation {
    // Pin conflict detection
    template<size_t N>
    constexpr bool hasDuplicatePins(const std::array<uint8_t, N>& pins) {
        for (size_t i = 0; i < N; i++) {
            for (size_t j = i + 1; j < N; j++) {
                if (pins[i] == pins[j]) return true;
            }
        }
        return false;
    }
    
    static_assert(!hasDuplicatePins(Config::INSTRUMENT_PINS),
        "Error: Duplicate pins detected in PINS configuration");
    
    // Platform-specific pin validation
    #ifdef PLATFORM_ESP32
        template<size_t N>
        constexpr bool pinsValidForESP32(const std::array<uint8_t, N>& pins) {
            for (const auto pin : pins) {
                // ESP32 input-only pins: 34-39
                if (pin >= 34 && pin <= 39) return false;
                // Invalid pins
                if (pin > 39) return false;
            }
            return true;
        }
        static_assert(pinsValidForESP32(Config::INSTRUMENT_PINS),
            "Error: Invalid pins for ESP32. Pins 34-39 are input-only, max pin is 39");
    #endif
    
    // Device name validation
    static_assert(sizeof(DEVICE_NAME) <= 32,
        "Error: DEVICE_NAME exceeds 32 characters");
    
    // Instrument count limits
    static_assert(Config::NUM_INSTRUMENTS <= 32,
        "Error: Maximum 32 instruments supported (limited by distributor bitmask)");
}
```

### Runtime Validation (Optional)

```cpp
// Device.cpp - Runtime validation for dynamic settings

void Device::validateConfiguration() {
    // Check for adequate timer resolution
    if (Config::TIMER_RESOLUTION < 4) {
        Serial.println("WARNING: TIMER_RESOLUTION < 4 may cause instability");
    }
    
    // Verify pin initialization
    for (const auto pin : Config::INSTRUMENT_PINS) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
    
    Serial.printf("Device initialized: %s (ID: 0x%04X)\n", 
                  Config::DEVICE_NAME_STR, Config::DEVICE_ID_VAL);
    Serial.printf("Platform: %s, Instruments: %d\n",
                  getPlatformName(), Config::NUM_INSTRUMENTS);
}
```

## Example Configurations

### 1. Simple Floppy Drive Setup (Minimal)
```cpp
// config.h
#pragma once

#define DEVICE_NAME "Floppy Drives"
#define DEVICE_ID 0x0001
#define PINS 26,27,2,4,18,19,21,22

#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;
```

### 2. PWM Buzzers with LEDs
```cpp
// config.h
#pragma once

#define DEVICE_NAME "PWM Orchestra"
#define DEVICE_ID 0x0002
#define PINS 12,13,14,15,16,17,18,19

#include "instruments/PwmDriver.h"
using InstrumentType = PwmDriver;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;

#define ENABLE_LEDS 23, WS2812, 8  // Pin 23, WS2812 type, 8 LEDs
```

### 3. Complex Multi-Network Setup
```cpp
// config.h
#pragma once

#define DEVICE_NAME "Stepper Synth Pro"
#define DEVICE_ID 0x0010
#define PINS 2,4,5,12,13,14,15,16,17,18,19,21,22,23

#include "instruments/StepperSynth.h"
using InstrumentType = StepperSynth;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;

#include "networks/UdpNetwork.h"
using SecondaryNetwork = UdpNetwork;

#define ENABLE_LEDS 25, WS2812, 16
#define TIMER_RESOLUTION 8
#define MIN_NOTE 24
#define MAX_NOTE 96
```

### 4. Custom Instrument (Advanced)
```cpp
// config.h
#pragma once

#define DEVICE_NAME "Dulcimer Custom"
#define DEVICE_ID 0x0100
#define PINS 2,4,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33

#include "instruments/custom/Dulcimer.h"  // Custom instrument
using InstrumentType = Dulcimer;

#include "networks/BleNetwork.h"
using NetworkType = BleNetwork;

#define ENABLE_LOCAL_STORAGE  // Save settings to NVS
```

## Migration from V1.0

### Automatic Conversion Tool

```python
# convert_config.py - Converts V1.0 config to V2.0

def convert_v1_to_v2(old_config_path):
    """Convert V1.0 configuration file to V2.0 format."""
    
    config = parse_v1_config(old_config_path)
    
    output = ["// config.h - Auto-converted from V1.0", "#pragma once", ""]
    
    # Device identity
    output.append(f"#define DEVICE_NAME \"{config.get('name', 'MMM Device')}\"")
    output.append(f"#define DEVICE_ID {config.get('device_id', '0x0001')}")
    output.append("")
    
    # Pins
    pins = config.get('pins', [])
    output.append(f"#define PINS {','.join(map(str, pins))}")
    output.append("")
    
    # Instrument
    instr = config.get('instrument_type', 'FloppyDrive')
    output.append(f"#include \"instruments/{instr}.h\"")
    output.append(f"using InstrumentType = {instr};")
    output.append("")
    
    # Network
    net = config.get('network_type', 'SerialNetwork')
    output.append(f"#include \"networks/{net}.h\"")
    output.append(f"using NetworkType = {net};")
    
    return "\n".join(output)
```

### Manual Migration Guide

**Old V1.0 (FloppyDrives.h):**
```cpp
constexpr uint16_t SYSEX_DEV_ID = 0x001;
constexpr std::array<uint8_t,10> pins = {26,27,2,4,18,19,21,22,23,32};
#include "Instruments/Default/FloppyDrive.h"
using instrumentType = FloppyDrive;
```

**New V2.0 (config.h):**
```cpp
#define DEVICE_ID 0x0001
#define PINS 26,27,2,4,18,19,21,22,23,32
#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;
```

**Changes:**
1. `SYSEX_DEV_ID` → `DEVICE_ID` (more descriptive)
2. `std::array pins` → `#define PINS` (simpler syntax)
3. `instrumentType` → `InstrumentType` (consistent naming)
4. Path simplified: `Instruments/Default/` → `instruments/`

## Benefits Summary

### Usability
- ✅ **90% reduction in configuration lines** (40+ → 10-15 lines)
- ✅ **Single file** - no hunting across multiple files
- ✅ **Self-documenting** - clear structure and comments
- ✅ **No C++ expertise required** - simple macro definitions

### Safety
- ✅ **Compile-time validation** - catch errors before upload
- ✅ **Helpful error messages** - tell users exactly what's wrong
- ✅ **Pin conflict detection** - prevent hardware damage
- ✅ **Platform-specific checks** - ensure compatibility

### Flexibility
- ✅ **Easy to share** - single file contains complete config
- ✅ **Version control friendly** - clear diffs
- ✅ **Template for new devices** - copy and modify
- ✅ **Optional features** - add only what you need

### Performance
- ✅ **Zero runtime overhead** - all processed at compile-time
- ✅ **Optimized pin access** - constexpr arrays
- ✅ **No unused code** - only linked components included
- ✅ **Flash savings** - unused features not compiled

## Configuration Best Practices

### 1. Start with Example
Copy an existing config that's similar to your hardware:
- `configs/examples/FloppyDrives.h` - Floppy drive setup
- `configs/examples/PwmBuzzers.h` - PWM buzzer setup
- `configs/examples/StepperMotors.h` - Stepper motor setup

### 2. Test Incrementally
1. Configure basic settings (name, ID, pins)
2. Compile and verify
3. Add optional features one at a time
4. Test each addition

### 3. Document Custom Settings
```cpp
// config.h
#pragma once

// Custom timer resolution for precise pitch control
// Lower = more accurate, but less stable (min: 4, default: 40)
#define TIMER_RESOLUTION 8

// Note range optimized for floppy drives
// C2 (36) to B5 (83) provides best tone quality
#define MIN_NOTE 36
#define MAX_NOTE 83
```

### 4. Use Version Control
Keep your configs in git with descriptive names:
```
configs/
  my_floppy_orchestra_v1.h
  my_floppy_orchestra_v2_added_leds.h
  stage_performance_config.h
  testing_config.h
```

## Next Steps

With configuration simplified, we can now design:
- **Part 3**: Network architecture (polymorphic interface)
- **Part 4**: Instrument system (hybrid approach)
- **Part 5**: Distributor runtime management
- **Part 6**: Implementation strategy
