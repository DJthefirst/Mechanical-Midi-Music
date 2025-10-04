# Mechanical MIDI Music V2.0 Configuration System

## Overview

This directory contains the V2.0 configuration system designed to simplify device setup from 40+ lines to 10-15 lines while adding compile-time validation and backward compatibility.

## Key Features

- **Single file configuration** - All settings in one `config.h` file
- **Compile-time validation** - Catch errors before upload
- **V1.0 compatibility** - Existing configs still work
- **Self-documenting** - Clear structure with helpful comments
- **Platform abstraction** - Easy porting between platforms

## Usage

1. Copy an example config from `configs/examples/`
2. Customize for your hardware
3. Include as `config.h` in your main directory
4. Compile and upload

## Example

```cpp
// config.h - Complete device configuration
#pragma once

//=== Device Identity ===
#define DEVICE_NAME "Floppy Orchestra"
#define DEVICE_ID 0x0042

//=== Hardware ===
#define PINS_LIST { 26, 27, 2, 4, 18, 19, 21, 22, 23, 32 }

//=== Components ===
#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

#include "networks/SerialNetwork.h"
using NetworkType = SerialNetwork;
```

## Migration from V1.0

See documentation in `Docs/REDESIGN_PART2_CONFIG.md` for complete migration guide.