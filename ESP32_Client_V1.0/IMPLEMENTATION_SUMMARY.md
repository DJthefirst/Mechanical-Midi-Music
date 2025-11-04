# Implementation Summary: Teensy 4.1 and ESP32 Support with HWPWM and SWPWM

## Overview
Successfully implemented complete support for both Teensy 4.1 and ESP32 platforms with Hardware PWM (HWPWM) and Software PWM (SWPWM) instrument types. Local storage has been disabled as requested.

## Changes Made

### 1. Platform Detection (Config.h)
- Added Teensy 4.1 platform detection using `ARDUINO_TEENSY41` and `__IMXRT1062__` defines
- Set appropriate platform capabilities (HAS_HARDWARE_PWM, HAS_USB_MIDI)
- Configured platform type enum for proper device identification

### 2. Platform Enum (Constants.h)
- Added `Teensy41` to the `Platform` enum
- Positioned after ESP platforms for logical grouping

### 3. Teensy 4.1 Hardware PWM Implementation
**Files Created:**
- `src/Instruments/Base/HwPWM/Teensy41_HwPWM.h`
- `src/Instruments/Base/HwPWM/Teensy41_HwPWM.cpp`

**Features:**
- Uses Teensy's native `analogWriteFrequency()` for dynamic frequency control
- 8-bit PWM resolution with 50% duty cycle for audio output
- Supports 12 simultaneous PWM channels across different FlexPWM modules
- Frequency range: 10Hz - 20kHz
- Implements pitch bend support
- Includes instrument timeout tracking

### 4. Teensy 4.1 Software PWM Implementation
**Files Created:**
- `src/Instruments/Base/SwPWM/Teensy41_SwPWM.h`
- `src/Instruments/Base/SwPWM/Teensy41_SwPWM.cpp`

**Features:**
- Uses Teensy's `IntervalTimer` for high-precision software PWM
- Supports up to 16 simultaneous channels
- Configurable timer resolution (default 8µs)
- Vibrato support with modulation wheel control
- Pitch bend support
- Optimized interrupt-driven PWM generation

### 5. Platform Selector Updates
**Updated Files:**
- `src/Instruments/Base/HwPWM/HwPWM.h` - Added Teensy 4.1 platform detection
- `src/Instruments/Base/SwPWM/SwPWM.h` - Added Teensy 4.1 platform detection

Both now automatically select the correct implementation based on platform defines.

### 6. Configuration Files
**Created:**
- `src/Configs/Teensy41HwPwmCfg.h` - Teensy 4.1 Hardware PWM config
  - Device ID: 0x0041
  - 12 PWM channels
  - Pins: 2, 3, 4, 5, 6, 7, 8, 9, 10, 22, 23, 24
  - Serial MIDI support
  
- `src/Configs/Teensy41SwPwmCfg.h` - Teensy 4.1 Software PWM config
  - Device ID: 0x0043
  - 16 PWM channels
  - Pins: 2-12, 14-18
  - 8µs timer resolution
  - Serial MIDI support

**Updated:**
- `src/Configs/HwPwmCfg.h` - Disabled LOCAL_STORAGE as requested
- `src/Configs/SwPwmCfg.h` - Already had LOCAL_STORAGE disabled

### 7. PlatformIO Configuration
**Updated: platformio.ini**
- Added `[env:teensy41]` environment
- Platform: teensy
- Board: teensy41
- Framework: arduino
- Build flags: C++17 support
- Monitor speed: 115200 baud

## Pin Assignments

### Teensy 4.1 Hardware PWM (12 channels)
Pins: 2, 3, 4, 5, 6, 7, 8, 9, 10, 22, 23, 24
- Distributed across multiple FlexPWM modules for optimal performance
- All pins support native hardware PWM

### Teensy 4.1 Software PWM (16 channels)
Pins: 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18
- Can use any digital I/O pin
- More channels available due to software implementation

### ESP32 Hardware PWM (8 channels)
Pins: 2, 4, 18, 19, 21, 22, 23, 25
- Uses even LEDC channels only (0, 2, 4, 6, 8, 10, 12, 14)
- Each channel uses a unique timer to prevent interference

### ESP32 Software PWM (10 channels)
Pins: 2, 4, 18, 19, 21, 22, 23, 33, 32, 16
- Uses hardware timer interrupt for PWM generation
- 8µs timer resolution

## Key Implementation Details

### Hardware PWM Approach
- **ESP32**: Uses LEDC peripheral with dedicated channels
- **Teensy 4.1**: Uses FlexPWM modules with analogWriteFrequency()
- Both support dynamic frequency changes for MIDI note playback

### Software PWM Approach
- **ESP32**: Uses ESP32 hardware timer with ICACHE_RAM_ATTR optimization
- **Teensy 4.1**: Uses IntervalTimer for precise microsecond timing
- Both implement efficient interrupt-driven PWM generation

### Common Features Across All Implementations
- MIDI note on/off support
- Pitch bend with real-time frequency adjustment
- Instrument timeout tracking
- Velocity handling
- Channel tracking for distributor support
- Active note management
- Reset and stop all functionality

## Configuration Selection
To use a specific configuration, set in `Config.h`:
```cpp
#define DEVICE_CONFIG "Configs/Teensy41HwPwmCfg.h"  // Teensy HW PWM
#define DEVICE_CONFIG "Configs/Teensy41SwPwmCfg.h"  // Teensy SW PWM
#define DEVICE_CONFIG "Configs/HwPwmCfg.h"          // ESP32 HW PWM
#define DEVICE_CONFIG "Configs/SwPwmCfg.h"          // ESP32 SW PWM
```

## Build Environments
- **ESP32**: Use `env:esp32dev` in PlatformIO
- **Teensy 4.1**: Use `env:teensy41` in PlatformIO

## Notes
- Local storage has been disabled across all ESP32 configurations as requested
- All implementations follow the existing architecture and coding patterns
- Timeout tracking is implemented for all instrument types
- Both platforms support serial MIDI communication
- Teensy 4.1 also supports native USB MIDI
