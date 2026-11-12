# Code Review and Remediation Report

## Scope

This review covered the active firmware under `src/`, all selectable PlatformIO instrument configurations, local persistence, MIDI and SysEx routing, distributor ownership, timer-driven instrument implementations, and the build-matrix script.

The review prioritized memory safety, malformed remote input, hardware array bounds, persistence capacity, real-time behavior, and reproducible builds. Candidate findings that did not hold under source inspection, including unsigned `millis()` rollover and intentional LED hue wrapping, were excluded.

## Remediation Summary

| Severity | Finding | Resolution |
| --- | --- | --- |
| Critical | Distributor moves invalidated strategy and active-note ownership pointers | `DistributorManager` now owns non-movable distributors through `std::unique_ptr`, and ownership tracking uses `const Distributor*` instead of `void*`. |
| Critical | Serial and USB produced different SysEx framing | Every transport now supplies complete `F0 ... F7` packets. GET/SET selection uses validated payload length. |
| Critical | A remote 14-bit distributor ID could allocate thousands of objects | Runtime and persistence share `MAX_DISTRIBUTORS = 64`; larger IDs are rejected and return an explicit error. |
| High | MIDI and SysEx handlers indexed malformed packets | MIDI commands have message-specific minimum lengths. SysEx requires a complete frame and command-specific payload lengths. |
| High | Serial MIDI parsing depended on currently available bytes | `NetworkSerial` now keeps parser state across loops and supports expected lengths, running status, real-time bytes, complete SysEx, and oversized-SysEx discard. |
| High | Configured voices could exceed pin/register arrays | Compile-time assertions cover PWM, direction, and shift-register capacities. Board-specific voice counts were corrected. Public instrument entry points also enforce runtime bounds. |
| High | Direct SysEx instrument IDs used the 32-slot protocol maximum | Instrument, note, velocity, and channel values are validated against the selected hardware configuration and MIDI limits. |
| High | Teensy persistence could exceed EEPROM and accepted corrupt records | Storage is capped at 64 distributors. Schema 2 stores a CRC-8 with every distributor record and skips corrupt records at boot. |
| Medium | Dulcimer retriggers inflated the global active-note count | The count changes only on inactive-to-active and active-to-inactive transitions. |
| Medium | Advertised Example builds referenced a missing class | Example uses the maintained custom `PwmInstrument`, has named environments, and compiles for both boards. |
| Medium | Incomplete SysEx commands and reset methods looked successful | Unsupported aggregate commands return `SysEx::Error`; malformed distributor commands return typed errors; PWM and MultiPhase per-instrument resets now stop the selected output. |

## Detailed Changes

### Distributor ownership

Distributors have stable heap addresses for their entire lifetime. Copy and move operations are disabled because address identity is part of active-note ownership. Strategies still use the Strategy pattern, but their owning pointer can no longer be invalidated by vector growth. Removing a distributor stops notes owned by that distributor before destruction.

Manager APIs now use 16-bit IDs where the wire protocol carries a 14-bit value. Invalid removal and serialization requests no longer silently target the final distributor. Deserialized methods, flags, and note ranges are constrained to implemented and valid values.

### MIDI and SysEx boundary

`MidiMessage` starts with a zeroed buffer and exposes complete-frame and payload-length checks. Standard MIDI dispatch validates two-byte and three-byte message families separately. Teensy USB transmit also verifies message length before reading data bytes.

The serial parser now handles:

- Messages split across loop iterations.
- Program Change and Channel Pressure two-byte messages.
- Running status.
- Interleaved MIDI real-time messages.
- Complete SysEx packets including `0xF7`.
- Oversized SysEx packets by discarding through the terminator.

SysEx error packets use command `0x7F` and a two-byte payload containing the failed command and error code:

| Error | Value |
| --- | --- |
| Unsupported command | `0x01` |
| Invalid payload | `0x02` |
| Invalid distributor | `0x03` |

`DeviceConstructWithDistributors`, `GetAllDistributors`, and distributor polyphonic-note aggregation need a versioned multipart protocol to exceed the fixed 128-byte MIDI packet. They now return Unsupported instead of empty or misleading substitute data.

### Hardware and real-time behavior

Compile-time checks ensure selected PWM pin arrays, direction pin arrays, and shift-register output counts cover the configured voices. Counts that differ by board are defined in board sections instead of being redefined globally.

StepSwShift no longer bit-bangs a shift register, including microsecond delays, from its timer callback. Direction changes are marked pending, the affected voice pauses, and `periodic()` latches the new direction before pulses resume.

Public HwPWM, SwPWM, MultiPhase, and Dulcimer methods reject invalid instrument indices. Methods that index MIDI tables also reject invalid notes and channels.

### ESP32 pin-map corrections

Definite ESP32-DevKit defects were removed: nonexistent GPIO 28, flash-reserved GPIO 6/7, input-only GPIO 36, UART conflicts, and internal component/LED collisions. The following configurations require wiring to match their updated maps:

- FloppySynth
- PneumaticValves
- Example
- FloppyDrive
- StepSw
- StepSwShift
- Teensy41HwPwm ESP32 variant
- Teensy41SwPwm ESP32 variant

The eight-drive FloppyDrive and StepSw examples still use GPIO 5 and 15 because they require 16 simultaneous outputs. These are valid output pins but ESP32 boot-strapping pins; attached hardware must not force an incompatible level during reset.

### Persistence compatibility

Storage schema version is now 2. Version 1 data is reset to defaults because old records have no checksum from which safe migration can be established.

Teensy records occupy 25 bytes: a 24-byte distributor construct plus CRC-8. With 64 records, the distributor region ends at `0x065B`, safely inside the Teensy 4.1 EEPROM allocation. ESP32 NVS stores the same logical record format in blobs. Record reads report validity, and corrupt records are not instantiated.

ESP32 may still need to erase NVS when the framework reports `ESP_ERR_NVS_NO_FREE_PAGES` or `ESP_ERR_NVS_NEW_VERSION_FOUND`; that recovery behavior is required by NVS and can lose saved configuration.

### Build and test automation

The matrix script gives every selected configuration its own `.pio/build/matrix/<configuration>` build directory and `.pio/libdeps/matrix/<configuration>` dependency directory. This removes the shared-directory race that produced false Pneumatic failures and Windows dependency-cleanup warnings. Generated `platformio.selected.*.ini` files are deleted in `finally` cleanup.

GitHub Actions now runs:

- Native protocol and CRC tests on Ubuntu.
- The complete selectable firmware matrix on Windows.

Native tests cover complete and incomplete SysEx frames, source/destination decoding, maximum payload truncation, and checksum corruption detection. Hardware timing and GPIO behavior still require on-device tests.

## Validation

Focused PlatformIO builds passed for:

- Dulcimer on Teensy 4.1 and ESP32.
- Example on Teensy 4.1 and ESP32.
- HwPWM, SwPWM, MultiPhase, StepSw, and StepSwShift on both boards.
- All eight ESP32 configurations whose pin maps changed.
- PneumaticValves on both boards.

The updated matrix harness passed all 26 selectable configurations: 13 instrument configurations on both Teensy 4.1 and ESP32. This includes the repaired Example targets and both PneumaticValves targets that previously failed because of shared build-directory churn.

The native tests could not run on the local Windows machine because no `gcc`, `g++`, Clang, or MSVC compiler is installed. The Linux CI job supplies GCC and runs the suite.

## Remaining Limitations

- DIN MIDI and ESP32 USB MIDI remain explicitly unsupported platform transports; they were outside the confirmed review defects.
- Aggregate SysEx operations require a future multipart/pagination protocol before implementation.
- GPIO behavior, boot-strapping levels, timer jitter, and physical note actuation require bench validation on each wired instrument.
- The existing PlatformIO dependency graph emits third-party ESP32 I2S deprecation warnings; no project-owned compiler error is associated with them.
