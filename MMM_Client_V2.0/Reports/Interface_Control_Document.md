# Mechanical Midi Music Interface Control Document

| Document field | Value |
| --- | --- |
| Document ID | MMM-ICD-001 |
| Revision | 1.1, 2026-11-11 |
| Subject | Reimplementation, current-system interoperability, modular firmware and platform services |
| Reviewed baseline | Repository HEAD `bc846ec6be9ecd9e4dd0610c8aeb4515a72ccbe7` **plus the staged, unstaged and untracked working-tree source present during review** |
| Current firmware identification | Device construct firmware value `2`; this is not a negotiated protocol version |
| Status | Source-derived compatibility baseline plus owner-supplied MIDI-like command map and specified replacement add-ons; not a hardware certification |

## 1. Purpose, scope and interpretation

This document specifies enough of the active firmware's externally visible behavior and internal responsibilities to implement a replacement without depending on its source structure. The replacement must communicate with existing MMM clients and firmware, retain PlatformIO, support independently selectable transports and add-ons, and reuse low-level MCU-specific services across different instruments and applications.

The review covered the active firmware, all 13 product configuration files and their two board variants, the build script, native tests, USB descriptor customization, local storage, existing reports, repository README, and historical extension material. Historical networks and archived instrument interfaces were inspected to distinguish former experiments from active functionality. Bundled dependencies, generated `.pio` outputs and archived implementations are not independent supported products.

The separately hosted GUI, protocol spreadsheet, wiki, deployed binaries, wiring and real hardware were **not** available as verified interoperability evidence. The GUI is referenced by the [repository README](../../README.md), but its source is not in this repository. Accordingly, "current" below means the reviewed working tree, not a claim about every previously deployed device. Revision 1.1 incorporates the owner's command map and configuration/add-on requirements supplied on 2026-11-11 without claiming those additions are already implemented.

### 1.1 Three distinct contracts

* **CURRENT:** Observed source behavior, including limitations. Implement a compatibility adapter for this behavior where host interoperability requires it.
* **OWNER PROTOCOL:** The supplied MMM command allocation. It is MIDI-like, **not official MIDI**. Where it conflicts with the source baseline, both are recorded and the owner allocation governs the replacement protocol profile.
* **REQUIRED:** Requirements for a replacement conforming to this ICD. `SHALL` indicates an acceptance requirement; `SHOULD` indicates a recommendation.
* **PROPOSED:** New architecture/configuration design, not an API or wire extension already implemented. Do not advertise these capabilities to legacy hosts until implemented and tested.

Do not reproduce unsafe actuator behavior merely to match a defect. A replacement SHALL preserve valid legacy packet layouts and identifiers, document intentional behavioral changes, and isolate any necessary legacy quirks in an explicitly selected compatibility policy. Safety takes precedence over exact defect emulation.

### 1.2 Reading guide

1. Sections 2-3 establish the system and build boundaries.
2. Sections 4-7 define transport, MIDI and exact MMM SysEx interoperability.
3. Sections 8-10 define reusable routing, instruments and platform services.
4. Sections 11-12 define add-ons and persistence.
5. Sections 13-16 define migration, verification, unresolved evidence and traceability.
6. Appendix A records the actual product/board profiles; Appendix B provides the fixed Dulcimer maps.

### 1.3 C++17 and separate legacy-controller edition

**CPP-01:** This ICD targets common C++17-capable microcontrollers and their supported embedded toolchains, including ESP32 and Teensy 4.1. The shared codebase SHALL use the common C++17 language/library subset supported by every declared target; it SHALL not require desktop filesystem/threading services, exceptions, RTTI, or heap allocation in real-time paths. Any optional library feature must have a documented target capability and bounded embedded implementation. A `-std=gnu++17` flag alone does not establish library availability.

**CPP-02:** PlatformIO SHALL select the toolchain/framework for each supported platform and compile the shared core as C++17. Low-level platform services remain independently reusable as specified in section 10. Tests and build profiles SHALL establish the actual supported language/library subset.

**CPP-03:** A separate **legacy-controller ICD edition** will cover Arduino Uno/Mega and other controllers without full C++17 support. It will preserve the configuration-section hierarchy and interoperable wire representation, while separately specifying language restrictions, fixed-capacity containers, resource limits and reduced capabilities. Uno/Mega are examples of that edition's platform blocks, not supported implementations or acceptance targets of this C++17 edition. A single protocol contract does not imply identical voice/file capacities on legacy hardware.

## 2. System boundaries and lifecycle

### 2.1 Runtime composition

```text
Host / DAW / MIDI source
        |
        +-- serial adapter --+
        +-- USB MIDI adapter +--> complete MidiMessage + ingress identity
        +-- future adapter --+               |
                                      MessageRouter
                                      /           \
                              MidiMsgHandler   SysExMsgHandler
                                    |             |
                              DistributorManager / Device state / Storage
                                    |
                              DistributionStrategy
                                    |
                              Instrument controller
                                    |
                     reusable waveform / step / actuator engines
                                    |
                     platform PWM / timer / GPIO / shift / storage
                                    |
                            external driver hardware

Instrument events ------------------> optional LED / diagnostic add-ons
```

CURRENT: There is one selected concrete controller per firmware image. It can expose multiple logical instrument slots, but the application does not dynamically load multiple unrelated controller types. Networks are compile-time selected and instantiated together. Add-ons are also compile-time selected.

REQUIRED:

* **ARCH-01:** Core protocol, routing, note ownership and configuration logic SHALL not depend on a particular network or peripheral SDK.
* **ARCH-02:** Adding a transport SHALL require an adapter, factory registration and configuration, not changes to note allocation or instrument code.
* **ARCH-03:** Adding an LED or other observer SHALL not require modifying MIDI decoding or low-level waveform generation.
* **ARCH-04:** Platform services SHALL be usable without MMM device state, distributors, MIDI note tables or LEDs.
* **ARCH-05:** A product SHALL compose reusable engines and platform backends rather than copying a complete instrument implementation for each MCU.
* **ARCH-06:** Compile-time selection SHALL remain available to avoid unnecessary code, memory and runtime dispatch on constrained MCUs.

### 2.2 Current startup and loop ordering

Source: [application entry point](../src/main.cpp).

1. Instantiate `InstrumentController<INSTRUMENT_TYPE>`. Its concrete constructor initializes hardware and may start a timer; many constructors delay 500 ms, and derived constructors may add another 500 ms.
2. Set runtime device instrument-type metadata from the controller.
3. Instantiate the distributor manager and both message handlers.
4. Create the configured network adapters and router; attach change callbacks.
5. Begin all networks, then delay 100 ms.
6. If enabled, load runtime configuration from local storage.
7. Reset all instruments.
8. Send `DeviceReady` from the runtime device ID to server address `0x3FFF` on every configured network.
9. Each loop polls each network for at most one returned message, then calls the controller's `periodic()`.

There is no default active distributor in the startup code. Without restored or host-created distributors, ordinary Note On/Off messages do not reach an instrument. Direct SysEx control does not require a distributor.

### 2.3 Replacement lifecycle

**LIFE-01:** Initialization SHALL establish inactive outputs before receiving executable control messages. Recommended states:

```text
Boot -> ValidateProfile -> EstablishSafeOutputs -> InitializeServices
     -> RestoreConfiguration -> StartTransports -> Ready
Any initialization failure -> FaultSafe
Ready -> Reconfigure (stop affected voices, validate, commit) -> Ready
Ready / Reconfigure / FaultSafe -> PanicAllOff
```

**LIFE-02:** State restoration SHALL not invoke normal persistence-writing setters. Transport initialization SHALL not block safety service indefinitely. A network reconnect SHALL not implicitly energize outputs.

**LIFE-03:** `periodic()` SHALL service non-ISR work, deferred output updates, timeouts and add-on flushing. Safety-critical maximum-on-time SHALL have an independent cutoff where a stalled loop can cause unacceptable heating or motion.

## 3. PlatformIO and consolidated configuration

### 3.1 Current build interface

Sources: [PlatformIO entry point](../platformio.ini), [configuration processing](../src/Config.h), [product profiles](../src/config), [build matrix script](../scripts/test_instrument_compilation.ps1).

* `default_envs = selected`; `[env:selected]` extends a board section and a product/board section.
* Board sections are `teensy41` and `esp32dev`; both use Arduino.
* `extra_configs` imports `DJthefirst`, `Example` and `User` INI files.
* There are 26 named product/board environments, plus `selected` and `native_tests`.
* Product configuration supplies C++17, device and instrument macros, selected networks, component flags, extras, dependencies and monitor speed.
* Board variants append pin maps and timer resolution, sometimes also instrument count.
* `CFG_INSTRUMENT_TYPE_VALUE` identifies the header included by the application. That header supplies `INSTRUMENT_TYPE`, often through an MCU dispatcher/alias.
* FastLED is listed as `fastled/FastLED@^3.5.0` in many profiles, even when LEDs are off. The platform and dependency versions are not fully pinned.
* Serial baud and monitor speed are separately repeated; configured serial is 115200 in all current products.

From the firmware directory:

```powershell
pio run -e selected
pio run -e Dulcimer_teensy41
pio run -e PneumaticValves_esp32dev
pio run -e selected -t upload
pio test -e native_tests
.\scripts\test_instrument_compilation.ps1
```

The script discovers active and commented `extends` lines inside `selected`, creates a temporary INI per selection, and isolates build and dependency directories. It cleans those temporary INIs in `finally`. A future registry must preserve equivalent matrix coverage rather than making commented selector lines the only test inventory.

The [workflow file](../.github/workflows/platformio-matrix.yml) describes native tests on Ubuntu and a build matrix on Windows. It is located beneath the firmware directory, not the repository-root `.github/workflows` directory. GitHub Actions discovery and command working directories need to be explicitly wired at repository root before relying on this as CI.

### 3.2 Current configuration fields

| Field/family | Default or interpretation | Owner in replacement |
| --- | --- | --- |
| `CFG_DEVICE_NAME`, `CFG_DEVICE_ID` | `"New Device"`, `0`; runtime overrides exist | Product identity / provisioning |
| `CFG_NUM_INSTRUMENTS`, `CFG_NUM_SUBINSTRUMENTS` | `1`, `1`; product <= 32 | Product logical topology |
| `CFG_MIN_NOTE`, `CFG_MAX_NOTE` | `0`, `127`; reported device range | Instrument profile |
| `CFG_TIMER_RESOLUTION_US` | `8`; board profiles normally use 1 or 8 | Platform timing profile |
| `CFG_NOTE_TIMEOUT_MS` | `0`; interpretation differs by instrument | Explicit per-engine timeout policy |
| `CFG_INSTRUMENT_*`, `CFG_INSTRUMENT_TYPE_VALUE` | Concrete class and compilation gates | Product engine selection |
| `CFG_MMM_NETWORK_*` | Serial, USB, DIN; other flags are not factory-wired | Transport selection |
| `CFG_MMM_NETWORK_SERIAL_BAUD` | Required when serial is selected | Transport settings |
| `USB_MIDI_SERIAL`, `CFG_USB_MIDI_PID` | Teensy USB class and optional PID | Board/product USB settings |
| `CFG_COMPONENT_*` | PWM, shift register, multiphase, limits | Derived component dependencies |
| `CFG_PINS_INSTRUMENT_PWM`, `..._DIR` | Ordered GPIO lists | Board/product wiring |
| `CFG_PINS_INSTRUMENT_ShiftRegister[1/2]` | Data, clock, latch, optionally enable, reset | Board/product wiring |
| `CFG_SHIFTREGISTER_NUM_OUTPUTS` | Compile-time chain width | Wiring/component profile |
| `CFG_SHIFTREGISTER_DATA_HOLDTIME_NS` | Used by Teensy software shift; default backend uses microsecond delays | Backend timing constraint |
| `CFG_PWM_TYPE`, `CFG_SHIFTREGISTER_TYPE`, `CFG_MULTIPHASE_TYPE` | Symbolic choices; not all are effective/implemented | Validated backend enum |
| `CFG_PWM_NOTES_DOUBLE`, `CFG_VIBRATO_ENABLED` | Some drivers check differently named macros | Canonical engine feature settings |
| `CFG_MULTIPHASE_WAVE_TABLE*`, `..._ENABLE_PIN` | Dimensions/table; current enable handling is not conditional | Multiphase topology |
| `CFG_LIMITS_POS_MIN/MAX` | Defaults 0/150; stepping code uses limits even without component flag | Motion profile |
| `CFG_EXTRA_LOCAL_STORAGE` | Restore and persistence hooks | Optional persistence service |
| `CFG_EXTRA_ADDRESSABLE_LEDS`, `CFG_NUM_LEDS`, `CFG_PIN_LED_DATA`, `CFG_BRIGHTNESS`, `CFG_LED_TYPE`, `CFG_COLOR_ORDER` | FastLED configuration | LED add-on and wiring |
| `CFG_UPDATES_PER_SECOND` | Present in INIs, not enforced by current LED implementation | Explicit add-on refresh limit |

CURRENT assertions require positive counts, product <= 32, positive timer resolution and an ordered 0-127 note range. Several drivers additionally check PWM/direction pin and register capacity. These checks do not establish pin validity, electrical safety, peripheral independence or timing accuracy.

### 3.3 Proposed consolidation without replacing PlatformIO

**CFG-01:** Retain INI environments, `extends`, `extra_configs` and named `pio run -e ...` targets. Consolidate repeated settings, not hardware-specific implementation.

The user-facing sections SHALL remain:

```text
Device Configuration
Instrument Configuration
Components/Utility Configuration
Network Configuration
Extras/Addons Configuration
Platform Configurations (one block per platform)
    Main Configuration
    Hardware Configurations
```

Each platform block consumes the shared Device, Instrument, Components/Utility, Network and Extras/Addons definitions. Its **Main Configuration selects which networks and add-ons are used**, composes their flags/dependencies and selects the platform implementation. Its **Hardware Configurations** supply the board/product pin maps, buses, timing and peripheral allocation. Selecting a platform must not require copying all shared definitions.

| Section | Single source of truth | Must not contain |
| --- | --- | --- |
| Device Configuration | Product name, address and device defaults | Wiring or network selection |
| Instrument Configuration | Engine/class, groups/slots, musical range and timeout policy | MCU register code or board pins |
| Components/Utility Configuration | Reusable PWM, timer, shift, math and bus requirements; backend options | Duplicated musical routing |
| Network Configuration | Available transport definitions, flags, settings and dependencies | Choice of enabled networks for every platform |
| Extras/Addons Configuration | LED, storage, SD and MidiPlayer definitions, settings and dependencies | Choice of enabled add-ons for every platform |
| Platform Configurations / Main Configuration | Framework/toolchain/C++17 options; selected networks/add-ons; composition of all five sections | Hardcoded GPIO behavior inside the core |
| Platform Configurations / Hardware Configurations | Pins, built-in/external SD interface, buses, timers and resource groups | Duplicated wire constants or shared add-on behavior |

Common compiler/warning options may be reused as internal helpers, but SHALL not replace these named sections. The resolved typed profile is generated/derived once from them, not an additional editable configuration source.

PROPOSED INI pattern, illustrative rather than a drop-in replacement:

```ini
; ---------- Device Configuration ----------
[Valves]
device_cfg =
    -D CFG_DEVICE_NAME="\"Pneumatic Valves\""
    -D CFG_DEVICE_ID=0x0018

; ---------- Instrument Configuration ----------
instrument_cfg =
    -D CFG_INSTRUMENT_PNEUMATICVALVESSW
    -D CFG_INSTRUMENT_SWPWM
    -D CFG_INSTRUMENT_TYPE_VALUE="\"Instruments/DJthefirst/PneumaticValvesSw.h\""
    -D CFG_NUM_INSTRUMENTS=8
    -D CFG_NUM_SUBINSTRUMENTS=1
    -D CFG_NOTE_TIMEOUT_MS=10000

; ---------- Components/Utility Configuration ----------
utility_cfg =
    -D CFG_COMPONENT_PWM

; ---------- Network Configuration ----------
network_serial =
    -D CFG_MMM_NETWORK_SERIAL
    -D CFG_MMM_NETWORK_SERIAL_BAUD=115200
network_usb =
    -D CFG_MMM_NETWORK_USB
    -D USB_MIDI_SERIAL

; ---------- Extras/Addons Configuration ----------
addon_storage =
    -D CFG_EXTRA_LOCAL_STORAGE
addon_sd =
    -D CFG_EXTRA_SD_CARD
addon_player =
    -D CFG_EXTRA_MIDI_PLAYER

; ---------- Platform Configurations: Teensy 4.1 ----------
[Valves_teensy41]
; Main Configuration: select networks and add-ons HERE.
platform = teensy
board = teensy41
framework = arduino
build_unflags = -std=gnu++11
main_cfg =
    -std=gnu++17
    ${Valves.device_cfg}
    ${Valves.instrument_cfg}
    ${Valves.utility_cfg}
    ${Valves.network_serial}
    ${Valves.network_usb}
    ${Valves.addon_storage}
    ${Valves.addon_sd}
    ${Valves.addon_player}
monitor_speed = 115200
; Add selected add-on/transport dependencies in lib_deps as needed.

; Hardware Configurations
hardware_cfg =
    -D CFG_TIMER_RESOLUTION_US=1
    -D CFG_PINS_INSTRUMENT_PWM="1,4,3,2,5,6,7,8"
    -D CFG_SD_BACKEND_TEENSY41_NATIVE
build_flags =
    ${Valves_teensy41.main_cfg}
    ${Valves_teensy41.hardware_cfg}

; ---------- Platform Configurations: ESP32 ----------
[Valves_esp32dev]
; Main Configuration: choose serial, storage, SD and player; no USB.
platform = espressif32
board = esp32dev
framework = arduino
build_unflags = -std=gnu++11
main_cfg =
    -std=gnu++17
    ${Valves.device_cfg}
    ${Valves.instrument_cfg}
    ${Valves.utility_cfg}
    ${Valves.network_serial}
    ${Valves.addon_storage}
    ${Valves.addon_sd}
    ${Valves.addon_player}
monitor_speed = 115200

; Hardware Configurations
hardware_cfg =
    -D CFG_TIMER_RESOLUTION_US=8
    -D CFG_PINS_INSTRUMENT_PWM="4,13,14,16,17,18,19,21"
    -D CFG_SD_BACKEND_ESP32_EXTERNAL_SPI
; Supply validated SD SPI bus, SCK/MISO/MOSI/CS and bus-clock settings.
; No external-reader pin defaults are assumed by this example.
build_flags =
    ${Valves_esp32dev.main_cfg}
    ${Valves_esp32dev.hardware_cfg}

[env:valves_teensy41]
extends = Valves_teensy41

[env:valves_esp32dev]
extends = Valves_esp32dev
```

The SD/player/backend flag names above are new configuration proposals, not existing macros in the reviewed firmware. The ESP32 example is intentionally incomplete until external-reader wiring is provided and validated; the build SHALL fail rather than guess pins. Exact platform/framework versions and selected dependencies also need pinning. An Arduino Mega/Uno block in the separate legacy edition follows the same Main/Hardware structure but consumes that edition's compatible utility/add-on implementations and compiler settings.

Explicit interpolation is preferable to assuming multiple inheritance concatenates list-valued settings. Shared sections define available modules; each platform's Main Configuration determines inclusion. Platform hardware options can change backend/timing without changing SD indexing, MidiPlayer control or the instrument's musical policy.

**CFG-02:** Initially map canonical configuration to the existing `CFG_*` macros to minimize migration risk. Then convert it once into a typed `constexpr` profile. Avoid introducing both an INI and a JSON/YAML file containing independently editable copies of the same values.

**CFG-03:** Reject contradictory engine flags, missing required services, unsupported transports/backends, duplicate GPIO assignments, insufficient chains and incompatible peripheral resource sharing. Validate pins against the exact MCU/board, not merely an array length. Intentional sharing requires an explicit ownership rule.

**CFG-04:** Derive component requirements from engine selection. Use defined numeric enums, not preprocessor comparisons between undefined tokens such as `DEFAULT`, `HW_DEFAULT` and `HW_HWACCEL`, which can all evaluate to zero.

**CFG-05:** Produce a resolved-profile/build report listing effective flags, pins, backend, resource groups, dependency versions and firmware identity. Preserve the existing 26 environment names as aliases during migration.

**CFG-06:** Hardware wiring is immutable at runtime unless a separately designed safe reconfiguration API exists. Legacy device-construct setters must not be interpreted as permission to change MCU or engine implementation.

**CFG-07:** Enabling MidiPlayer SHALL require SD. The platform Main Configuration SHALL fail validation for missing dependencies rather than automatically enabling a silent substitute. Selecting SD SHALL require exactly one available platform SD backend and a valid Hardware Configuration.

## 4. Transport interfaces

### 4.1 Current transport contract

Source: [INetwork](../src/Networks/INetwork.h), [network manager/factory](../src/Networks/NetworkManager.h), [router](../src/MsgHandling/MessageRouter.cpp).

| Operation | Current semantics | Required replacement semantics |
| --- | --- | --- |
| `begin()` | Void; no common failure result | Return explicit initialization/availability result |
| `readMessage()` | Optional complete message; none when unavailable | Nonblocking poll, complete normalized message with ingress identity |
| `sendMessage(message)` | Void; sends/copies message as adapter defines | Distinguish accepted, queued, unavailable and failed; bounded buffering |
| `sendString(text)` | Serial text or USB serial debug; sometimes no-op | Separate diagnostic channel; never interleave text with MIDI data |

`MidiMessage` is a 128-byte zero-initialized buffer plus an 8-bit length. It represents ordinary raw MIDI bytes, including the status byte, or a complete `F0 ... F7` SysEx. Internal channels and instrument IDs are zero-based.

**NET-01:** Every adapter SHALL deliver the same normalized message representation. USB event packets, BLE timestamps and RTP/UDP session headers SHALL not leak into the core.

**NET-02:** Polling SHALL not wait for the rest of an incomplete stream. Each adapter owns its parser state and bounded queue. Queues SHALL preserve order within an ingress stream. Cross-transport simultaneous arrival order is not specified by the current protocol.

**NET-03:** Preserve ingress adapter/session identity until a reply is sent. Current query responses go only to the originating network; ordinary received MIDI is not automatically forwarded to other networks.

CURRENT: Change callbacks in startup call `broadcastDeviceChanged()` without an ingress argument. Consequently changes generate a notification on **all** networks, including the source. Although the router supports excluding a source, that exclusion is not wired into those callbacks. Removal/creation can produce multiple notifications. Hosts must treat them as refresh hints, not unique transaction acknowledgments.

**NET-04:** Bound service per adapter so a busy source cannot starve others or safety work. Report queue overflow and dropped/malformed traffic. Never treat a no-op unsupported adapter as a functioning network.

### 4.2 Serial MIDI profile

Source: [serial adapter](../src/Networks/NetworkSerial.cpp).

CURRENT:

* Uses Arduino `Serial` at configured baud, normally 115200. On Teensy this can be USB serial rather than physical DIN/UART.
* Sends exactly the stored MIDI bytes with no newline, length prefix, checksum or custom serial envelope.
* Preserves partial messages across calls and supports channel running status.
* Channel message lengths: `8n`, `9n`, `An`, `Bn`, `En` = 3; `Cn`, `Dn` = 2.
* System-common lengths: `F1`, `F3` = 2; `F2` = 3; `F6` = 1.
* `F8`-`FF` are returned immediately as one-byte real-time messages without disturbing an in-progress message.
* `F0` begins SysEx and clears running status. `F7` terminates it. Oversized SysEx is discarded; see the recovery caveat in section 13.
* Channel status replaces an incomplete non-SysEx message. System-common status clears running status.
* `sendString()` uses `Serial.println()` on the same channel; invoking it while carrying MIDI can corrupt host parsing.
* No partial-message timeout is implemented.

**NET-05:** Keep raw 115200 serial as the legacy profile. A separately selected DIN profile SHALL use the appropriate 31250-baud UART and physical interface; changing serial's baud alone does not implement DIN electrical signaling.

**NET-06:** A replacement stream parser SHALL define bounded partial-frame lifetime, handle interleaved real-time messages, and resynchronize on unexpected non-real-time status. Preserve valid running-status behavior. Do not introduce an undocumented wire envelope.

### 4.3 Teensy USB MIDI profile

Sources: [USB adapter](../src/Networks/NetworkUSB/Teensy41_NetworkUSB.cpp), [product name](../src/Networks/NetworkUSB/Teensy41_USBName.c), [PID override](../src/Networks/NetworkUSB/Teensy41_USBPid.c).

CURRENT:

* USB MIDI is provided by the Teensy framework; `USB_MIDI_SERIAL` enables the combined class.
* Receive converts API channels 1-16 to raw status channels 0-15, reconstructs 2/3-byte channel messages and copies the framework's SysEx array up to 128 bytes.
* Transmit converts channels back to 1-16. Pitch bend converts unsigned 14-bit value to the SDK's signed value by subtracting 8192. It flushes using `send_now()`.
* Product descriptor name is compiled from `CFG_DEVICE_NAME`; the initializer expands 32 characters. Runtime SysEx renaming does not change USB enumeration identity.
* Optional `CFG_USB_MIDI_PID` patches product-ID bytes before `usb_init()`; no custom VID or per-device USB serial-number policy is defined here.
* The active dispatcher substitutes an inert USB adapter on unsupported platforms, including `esp32dev`. Some ESP32 products therefore compile with USB selected but have no working USB MIDI transport.

**Compatibility defect:** Current transmit removes `F0` and calls the SDK with `hasTerm=true`. The installed Teensy core selects its already-terminated-buffer path and does not add `F0`. Thus the raw output is not a conforming complete SysEx frame. A replacement SHALL send the complete `F0 ... F7` buffer in the already-framed mode, or send only the interior in the SDK's add-framing mode. Verify host-visible bytes, not the adapter's comments or successful compilation.

Oversized/chunked SysEx and USB real-time receive semantics require explicit adapter tests; the current generic non-SysEx receive reconstruction is not proof of correct system-message handling.

### 4.4 Unimplemented and historical network types

| Type | Reviewed status | Extension requirement |
| --- | --- | --- |
| DIN | Active header/implementation are placeholders | UART MIDI codec plus board-specific physical interface |
| UDP | Only [historical implementation](../LegacyCode/NetworkUDP.cpp); not registered by active factory | Define datagram framing, destination/session routing, ordering/loss and discovery |
| RTP MIDI | [Historical experiment](../LegacyCode/NetworkRTP.cpp); note callbacks, incomplete send paths | Reuse standard session/timing stack; normalize full supported message set |
| BLE MIDI | [Historical experiment](../LegacyCode/NetworkBLE.cpp); incomplete transmit and malformed callback status construction | Standard BLE MIDI framing/timestamps and bounded ingress queue |
| ESP32 USB MIDI | No active backend | Distinguish USB-capable MCU variants from USB-to-UART bridges |

No normative interoperable UDP port, BLE session or RTP extension is established by those experiments. Configuration contains multicast `224.5.6.7`, web port 80 and a field named OTA port with value 8337; these are not an active network contract. Do not promote embedded historical connection credentials into shared build configuration or this ICD.

## 5. MIDI-compatible event behavior within MMM

MMM is a custom **MIDI-like protocol, not official MIDI**. This section compares its source-derived event subset with MIDI conventions to make transport/file adapters unambiguous; it does not assert that all MMM commands, address conventions or extensions conform to MIDI. Standard MIDI files are an explicit input format for the MidiPlayer add-on in section 11.3, not the definition of the MMM control protocol.

Sources: [MIDI constants](../src/Constants.h), [MIDI dispatch](../src/MsgHandling/MidiMsgHandler.cpp), [distributor](../src/Distributors/Distributor.cpp).

| Raw MIDI | Current handling |
| --- | --- |
| `8n note velocity` | Note Off sent to all matching distributors |
| `9n note velocity` | Note On sent to all matching distributors; velocity 0 acts as Note Off after the distributor's range check |
| `An note pressure` | Direct controller key-pressure hook, bypasses distributor mapping |
| `Bn controller value` | Direct controller CC hook; then CC 120 and 123 call global `stopAll()` |
| `Cn program` | Direct controller hook; base is no-op |
| `Dn pressure` | Direct controller hook; base is no-op |
| `En lsb msb` | Direct pitch bend `(msb << 7) | lsb`; center `0x2000` |
| `F0 ... F7` | MMM SysEx handler when complete, manufacturer ID/address match |
| `FA` | Current code calls `stopAll()` even though MIDI defines this as Start |
| `FC` | Standard MIDI Stop, but current handler does not act on it |
| `FF` | `resetAll()` |
| Other system/common/realtime | No implemented musical action in the handler |

Minimum lengths are checked by message family, but complete validation of every raw data byte is not implemented.

CURRENT flags are partly metadata: global `Muted`, `OmniMode`, `DamperPedal` and `NumPolyphonicNotes` are stored/reported but do not generally enforce muting, omni routing, sustain or polyphony. CC 126/127 contain no active polyphony implementation; CC 121 does not perform a base reset. `Vibrato` is consulted by parts of software PWM, with inconsistent gates described in section 13.

**MIDI-01:** Support valid note messages, pressure/program/CC hooks and 14-bit pitch bend with explicit zero-based channel conversion. Optional musical effects SHALL be advertised only when implemented.

**MIDI-02:** The replacement's normal musical policy SHALL honor standard `FC` Stop; interpreting `FA` Start as all-off requires an explicit legacy behavior option, never an undocumented default.

**MIDI-03:** Panic/all-off SHALL stop actual outputs, clear note ownership and cancel pending actuator work. An engine's unsupported effect is distinct from malformed input.

**MIDI-04:** Velocity-zero normalization SHALL occur before new-note allocation. Direct SysEx Note On currently does not normalize zero velocity: PWM engines can start a note with it. Document and test the chosen compatibility policy.

## 6. MMM MIDI-like wire contract and source compatibility

### 6.1 Envelope and encoding

Source: [message representation](../src/MsgHandling/MidiMessage.h).

```text
F0 7D SrcAdr0 SrcAdr1 DestAdr0 DestAdr1 CMD PAYLOAD... F7
```

| Byte offset | Length | Meaning |
| --- | --- | --- |
| 0 | 1 | `F0`, SysEx start |
| 1 | 1 | `7D`, non-commercial/educational manufacturer ID |
| 2 | 2 | Source address, most-significant 7-bit group first |
| 4 | 2 | Destination address, most-significant 7-bit group first |
| 6 | 1 | Command ID |
| 7 | 0-120 | Command-specific payload |
| Last | 1 | `F7`, SysEx end |

**WIRE-01:** Legacy packets SHALL be at most 128 bytes including framing: 7 header bytes + up to 120 payload bytes + terminator. There is no wire CRC, length field, sequence number, transaction ID or authentication field.

Address bytes are named `Adr0` (high 7-bit group) and `Adr1` (low 7-bit group). They are not raw high/low 8-bit halves. The owner's address notation `0x7F7F` means the transmitted pair `7F 7F`; it decodes to internal 14-bit value `0x3FFF`, **not** scalar `0x7F7F`. The wire representation is unchanged.

14-bit address/flag encoding:

```text
encode14(v) = [(v >> 7) & 7F, v & 7F]
decode14(h, l) = (h << 7) | l
```

Fixed-width masks are big-endian groups of seven bits, not native-endian struct bytes:

```text
channels16(v)    = [(v >> 14) & 03, (v >> 7) & 7F, v & 7F]
instruments32(v) = [(v >> 28) & 0F, (v >> 21) & 7F,
                   (v >> 14) & 7F, (v >> 7) & 7F, v & 7F]
```

Mask bit 0 corresponds to channel 0 (MIDI channel 1) or instrument slot 0. Example: channels 1 and 16 in human numbering = `0x8001` = `02 00 01`; slots 0 and 31 = `0x80000001` = `08 00 00 00 01`.

**WIRE-02:** All interior bytes SHALL be 0-127 in a conforming packet. The current constructor and handler do not enforce that rule uniformly. Use 7-bit ASCII names for legacy interoperability; arbitrary UTF-8 is not a defined payload encoding.

### 6.2 Addressing and responses

* Owner notation `DevAddr 0x0000` = wire pair `00 00`, reserved for device broadcast.
* Owner notation `DevAddr 0x7F7F` = wire pair `7F 7F`, reserved for server; internal decoded value `0x3FFF`.
* Recommended provisioned device IDs = `1..0x3FFE`. CURRENT defaults permit 0 and several products share ID `0x0018`.
* The handler accepts only its current runtime ID or broadcast. Other addresses and other manufacturer IDs are ignored.
* A query reply uses the device ID captured on request entry as source, the request's source as destination, and normally the same command.
* Most successful setters have **no direct reply**. Change notifications use `04`, empty payload, device source and server destination.
* Startup emits `00`, empty payload, device source and server destination.
* There is no request/response direction bit. A host receiving a reply SHALL not reflect it back as another command.

**WIRE-03:** Legacy hosts SHALL serialize ambiguous management requests, especially same-command requests to different distributors, because most field replies omit distributor ID and there is no correlation token.

**WIRE-04:** After an ID change, use the new address for subsequent traffic. A change notification uses the new ID; it is not a correlated acknowledgment of the old address.

Shared-bus broadcast discovery has no collision-avoidance or response backoff. USB/point-to-point serial does not have the same electrical collision problem. Provision unique device IDs independently of product PID.

### 6.3 Source-baseline error packets and omission behavior

This subsection documents the **source-baseline profile only**. The owner command map reserves `7F` for Extend; it does not assign a standalone Error command. Do not use the two meanings interchangeably.

```text
F0 7D device_H device_L requester_H requester_L 7F FAILED_CMD ERROR_CODE F7
```

| Error code | Meaning |
| --- | --- |
| `01` | Explicitly unsupported command |
| `02` | Invalid distributor-command payload length |
| `03` | Invalid distributor ID/capacity |

CURRENT explicit unsupported responses exist for aggregate commands `20` and `32`. Unknown commands, including reserved extras, are generally ignored without a reply. Several device/instrument setters silently ignore short or invalid payloads; `52` returns zero even for an invalid instrument query. Do not assume every invalid request returns `7F`.

**WIRE-05:** Preserve valid legacy behavior. New management extensions SHALL provide explicit errors and correlation in a separately versioned/negotiated schema; do not insert extra fields into legacy replies.

### 6.4 Owner command map and profile separation

**OWNER-01:** The replacement's owner-protocol profile SHALL retain the following top-level allocations. "Reserved" has no executable semantics; "Extend" reserves a group's expansion point without defining its payload or dispatch mechanism.

| Range / CMD | Owner allocation |
| --- | --- |
| `00`-`04` | Device Ready; Reset Device Configuration; Discover Devices (Ping); Message (Strings); Device Changed (client -> server notify) |
| `05`-`0E` | Reserved |
| `0F` | Extend (system group) |
| `10`-`1E` | Reserved |
| `1F` | Extend (reserved group) |
| `20`-`25` | Device Construct with Distributors; Device Construct without Distributors; Device ID; Device Name; Device Boolean; Number of Polyphonic Notes |
| `26`-`2E` | Reserved |
| `2F` | Extend (device group) |
| `30`-`35` | Get Number of Distributors; Add/Set Distributor; Get All Distributors; Remove Distributor (ID); Remove All Distributors; Toggle Mute (ID) |
| `36`-`3E` | Reserved |
| `3F` | Extend (distributor management group) |
| `40`-`45` | Distributor (ID) Construct; Channels; Instruments; Distribution Method; Boolean Values; Min/Max Note Values |
| `46`-`4E` | Reserved |
| `4F` | Extend (distributor get/set group) |
| `50`-`55` | Reset All Instruments (Set Sub Device Mode); Reset Instrument (ID); Get Instrument (ID) Active Notes; Set Instrument (ID) Direct Message; Set Instrument (ID) Note On; Set Instrument (ID) Note Off |
| `56`-`5E` | Reserved |
| `5F` | Extend (instrument group) |
| `60` | Storage |
| `61` | LED |
| `62` | SD; subcommands specified in section 11.2 |
| `63` | Playback; MidiPlayer subcommands specified in section 11.3 |
| `64`-`6E` | Reserved |
| `6F` | Extend (extras/add-ons group) |
| `70` | Custom CMD 1, 1-byte value (possible LED pattern; no assigned behavior) |
| `71` | Custom CMD 2, 1-byte value |
| `72` | Custom CMD 3, 1-byte value |
| `73` | Custom CMD 4, 1-byte value |
| `74` | Custom CMD 5, 1-byte value |
| `75` | Custom CMD 6, 1-byte value |
| `76` | Custom CMD 7, 1-byte value |
| `77` | Custom CMD 8, 1-byte value |
| `78` | Custom CMD 9, 2-byte value |
| `79` | Custom CMD 10, 2-byte value |
| `7A` | Custom CMD 11, 2-byte value |
| `7B` | Custom CMD 12, 2-byte value |
| `7C` | Custom CMD 13, 4-byte value |
| `7D` | Custom CMD 14, 4-byte value |
| `7E` | Custom CMD 15, 8-byte value |
| `7F` | Extend (custom group), **not standalone Error** |

Custom value widths refer to payload bytes after the top-level command; values remain 7-bit safe. Numerical interpretation, get/set convention and meaning require a product-specific contract. They are not automatically implemented LED commands. `7D` in the command position is distinct from `7D` in the envelope ID position.

**OWNER-02:** Select the peer's protocol profile explicitly in provisioning/host configuration until a separately specified capability handshake exists. Do not infer a profile from `7F` payload length or mix it during a session. The existing firmware emits source-baseline `7F` errors; an owner-profile implementation reserves it for future extension. Owner-profile SD/player responses instead carry a status within `62`/`63`, as specified below.

The source calls `62` `ExtraSdPlayback` and has no `63` handler. The owner map separates SD (`62`) and Playback (`63`). New implementations SHALL use that separation. The wording "Set Sub Device Mode" for `50` and the extension payloads are not defined by the supplied map; the known `resetAll()` operation remains documented, and no additional mode payload is invented.

## 7. Source-baseline command behavior and constructs

These detailed request/reply tables retain the reviewed firmware behavior. Section 6.4 records the owner allocation, including commands unimplemented by that firmware. Sections 11.2-11.3 define new replacement SD/player bodies; they are not claims about deployed support.

Notation: `D` = 2-byte distributor ID; `I` = one-byte instrument slot; `C` = one-byte zero-based channel; `N` = note; `V` = velocity. Sizes are payload bytes, excluding framing. `>=` describes current minimum acceptance, not permission to append undocumented extension fields.

### 7.1 System and device commands

Source: [SysEx handler](../src/MsgHandling/SysExMsgHandler.cpp).

| CMD | Name | Request payload | Reply / current side effect |
| --- | --- | --- | --- |
| `00` | DeviceReady | Payload ignored | Same command, empty payload |
| `01` | ResetDeviceConfig | Payload ignored | No direct reply; clear distributors/storage; name becomes `"New Device"`; change notifications |
| `02` | DiscoverDevices | Payload ignored | Same command, 2-byte current device ID |
| `03` | Message | No active handler | Ignored |
| `04` | DeviceChanged | Notification only | No active inbound handler |
| `20` | DeviceConstructWithDistributors | Any | `7F 20 01`; no aggregate transfer |
| `21` | DeviceConstruct | Empty = GET; >=56 = SET | GET returns 56 bytes; SET updates mutable fields, persists subset, notifies |
| `22` | DeviceID | Empty = GET; >=2 = SET ID | GET returns 2 bytes; SET persists/notifies |
| `23` | DeviceName | Empty = GET; nonempty = SET | GET returns 20 zero-padded bytes; SET uses at most 19 bytes, trims trailing NULs, persists/notifies |
| `24` | DeviceBoolean | Empty = GET; >=2 = SET flags | GET returns 2 bytes; SET persists/notifies |
| `25` | DeviceNumPolyphonicNotes | No active handler | Ignored; value can be changed through construct byte 18 |

Short nonempty payloads for `21`, `22`, `24` have no mutation and no error response. Empty `23` cannot set an empty name; use a nonempty NUL payload if needed, subject to storage behavior.

Reset `01` is **not** a complete in-memory reset of ID, booleans, polyphony or instrument metadata. Removing all distributors does stop all instruments. Persistence reset and runtime reset are not identical.

### 7.2 Device construct: exactly 56 bytes

Source: [device serialization](../src/Device.h).

| Offset | Bytes | Field | Current GET/SET semantics |
| --- | --- | --- | --- |
| 0 | 2 | Device ID | Runtime ID; mutable |
| 2 | 1 | Instrument groups | Compile-time count; SET ignored |
| 3 | 1 | Subinstruments/group | Compile-time count; SET ignored |
| 4 | 1 | Reserved | GET zero |
| 5 | 1 | Instrument type | Runtime metadata; SET mutable, does not change compiled driver |
| 6 | 1 | Reserved | GET zero |
| 7 | 1 | Platform type | Compile-time enum; SET ignored |
| 8 | 1 | Reserved | GET zero |
| 9 | 2 | Firmware version | `00 02`; SET ignored |
| 11 | 1 | Reserved | GET zero |
| 12 | 1 | Minimum note | Compile-time metadata; SET ignored |
| 13 | 1 | Maximum note | Compile-time metadata; SET ignored |
| 14 | 2 | Device booleans | Mutable |
| 16 | 2 | Reserved | GET zero |
| 18 | 1 | Polyphonic-note limit | Mutable metadata: 0 unlimited, 1 mono, >1 limited; not generally enforced |
| 19 | 17 | Reserved | GET zero |
| 36 | 20 | Name | GET truncates/pads to 20; SET ends at first NUL or 20 bytes |

Device flags: bit 0 muted, bit 1 omni, bit 2 damper pedal, bit 3 vibrato. Getter reports these four flags; other bits have no active device state.

Instrument enum values:

| Value | Name |
| --- | --- |
| `00` | None |
| `01` | SW_PWM |
| `02` | HW_PWM |
| `03` | StepHW_PWM |
| `04` | StepSW_PWM |
| `05` | ShiftRegister |
| `06` | MultiPhase |
| `10` | StepperMotor |
| `11` | FloppyDrive |

Platform enum values: ESP32=1, ESP8266=2, Teensy41=3, Uno=4, Mega=5, Due=6, Micro=7, Nano=8. Enumeration is not implementation support. The maintained board matrix covers Teensy 4.1 and classic ESP32 `esp32dev`; AVR and other fallbacks are incomplete.

**DATA-01:** Replacement serializers SHALL use these offsets and explicit encoding, never a compiler-native C/C++ struct layout. Reserved GET bytes SHALL be zero. Read-only capabilities SHALL reflect the compiled profile, regardless of requested legacy metadata.

### 7.3 Distributor management and field commands

| CMD | Name | Request payload | Reply / current side effect |
| --- | --- | --- | --- |
| `30` | GetNumOfDistributors | Payload ignored | 1-byte count, 0-64 |
| `31` | AddDistributor | >=24-byte construct | ID-directed upsert; fills missing intermediate slots; no direct success reply |
| `32` | GetAllDistributors | Any | `7F 32 01`; query individually instead |
| `33` | RemoveDistributor | >=2: `D` | Remove existing index, compact later IDs, no direct success reply |
| `34` | RemoveAllDistributors | Payload ignored | Global all-off, clear list, no direct reply |
| `35` | ToggleMuteDistributor | >=2: `D` | Empty same-command reply; stops all controller notes even when unmuting |
| `40` | DistributorConstruct | Exactly 2 = GET `D`; >=24 = SET construct | GET returns 24 bytes including ID; SET upserts |
| `41` | DistributorChannels | Exactly 2 = GET `D`; >=5 = SET `D + channels16` | GET returns 3 mask bytes **without ID** |
| `42` | DistributorInstruments | Exactly 2 = GET `D`; >=7 = SET `D + instruments32` | GET returns 5 mask bytes **without ID** |
| `43` | DistributorMethod | Exactly 2 = GET `D`; >=3 = SET `D + method` | GET returns 1 byte **without ID** |
| `44` | DistributorBoolValues | Exactly 2 = GET `D`; >=4 = SET `D + flags14` | GET returns 2 bytes **without ID** |
| `45` | DistributorMinMaxNotes | Exactly 2 = GET `D`; >=4 = SET `D + min + max` | GET returns 2 bytes **without ID** |

Field commands require an existing distributor. A payload shorter than the required selector/field produces invalid-payload or invalid-distributor error according to validation order. Construct/upsert commands reject IDs >=64. There is no distributor-polyphony command in the active constant dictionary, despite unused helper declarations.

Defaults for a newly filled slot: no enabled channels, no enabled instruments, method RoundRobinBalance, min 0/max 127, flags zero. Every mutation persists when storage is enabled and emits change notifications. Auto-filling slots can emit several notifications.

### 7.4 Distributor construct: exactly 24 bytes

| Offset | Bytes | Field |
| --- | --- | --- |
| 0 | 2 | Distributor ID, 14-bit |
| 2 | 3 | Enabled MIDI channel mask, 16-bit |
| 5 | 5 | Enabled instrument-slot mask, 32-bit |
| 10 | 1 | Distribution method |
| 11 | 1 | Reserved zero |
| 12 | 1 | Minimum note |
| 13 | 1 | Maximum note |
| 14 | 2 | Boolean flags |
| 16 | 8 | Reserved zero |

Methods: StraightThrough=0, RoundRobin=1, RoundRobinBalance=2, Ascending=3, Descending=4, Stack=5. Stack is not implemented; current setters normalize 5 or other unsupported methods to 2. Flags: muted bit 0; note-overwrite bit 1. Note-overwrite is stored but allocation does not consult it. Note ranges are clamped/masked to 0-127 and swapped if reversed.

### 7.5 Direct instrument and extras commands

| CMD | Name | Request payload | Reply / current side effect |
| --- | --- | --- | --- |
| `50` | ResetAllInstruments | Payload ignored | No reply; controller `resetAll()` |
| `51` | ResetInstrument | >=1: `I` | No reply; valid slot reset |
| `52` | GetInstrumentNumActiveNotes | >=1: `I` | 1-byte count; invalid/missing slot yields zero |
| `53` | SetInstrumentDirectMessage | No handler | Ignored |
| `54` | SetInstrumentNoteOn | >=4: `I C N V` | No reply; direct controller play |
| `55` | SetInstrumentNoteOff | >=4: `I C N V` | No reply; direct controller stop |
| `60` | ExtraStorage | No handler | Ignored; persistence is implicit in other setters |
| `61` | ExtraLED | No handler | Ignored; no runtime LED configuration protocol |
| `62` | ExtraSdPlayback | No handler | Ignored; no active SD player |
| `63` | Playback (owner allocation) | No handler in reviewed source | Replacement MidiPlayer contract in section 11.3 |
| `7F` | Error | Reply command | No active inbound handler |

Instrument slots must be less than `CFG_NUM_INSTRUMENTS * CFG_NUM_SUBINSTRUMENTS`; channel 0-15, note/velocity 0-127. These direct commands bypass channel masks, distributor range/mute and ownership assignment. Current direct play does not reliably establish channel/owner tracking for later pitch bend or distributed Note Off.

### 7.6 Golden wire examples

All values are hexadecimal. Device wire pair `00 18`, requester/server wire pair `7F 7F` (owner notation `0x7F7F`, internal value `0x3FFF`). The `7F` Error example is source-baseline only.

```text
Broadcast discover:
F0 7D 7F 7F 00 00 02 F7

Discover reply:
F0 7D 00 18 7F 7F 02 00 18 F7

Read device construct:
F0 7D 7F 7F 00 18 21 F7
# Reply is the reversed-address header with command 21 and 56-byte payload.

Read distributor 0:
F0 7D 7F 7F 00 18 40 00 00 F7

Set distributor 0 channels to MIDI channels 1 and 16:
F0 7D 7F 7F 00 18 41 00 00 02 00 01 F7

Read those channels:
F0 7D 7F 7F 00 18 41 00 00 F7
# Reply contains only the three channel bytes:
F0 7D 00 18 7F 7F 41 02 00 01 F7

Direct note on: slot 0, MIDI channel 1, A4 (69), velocity 100:
F0 7D 7F 7F 00 18 54 00 00 45 64 F7

Matching direct note off:
F0 7D 7F 7F 00 18 55 00 00 45 00 F7

Request unsupported aggregate distributor transfer:
F0 7D 7F 7F 00 18 32 F7
# Error reply:
F0 7D 00 18 7F 7F 7F 32 01 F7

Device changed notification:
F0 7D 00 18 7F 7F 04 F7
```

Example 24-byte distributor construct: slot 0, MIDI channel 1, instrument slots 0-3, RoundRobinBalance, full note range, flags off:

```text
Offset 00: 00 00
Offset 02: 00 00 01
Offset 05: 00 00 00 00 0F
Offset 10: 02 00 00 7F 00 00
Offset 16: 00 00 00 00 00 00 00 00
```

## 8. Distribution and note-ownership interface

Sources: [manager](../src/Distributors/DistributorManager.cpp), [strategies](../src/Distributors/DistributionStrategies.cpp), [strategy state](../src/Distributors/DistributionStrategy.h).

### 8.1 Current allocation semantics

All matching channel distributors receive the same note event, in list order. Overlapping pools can retrigger/overwrite the same physical slot. There is no one-distributor-per-note guarantee.

| Method | Note On selection | Note Off selection |
| --- | --- | --- |
| StraightThrough | Slot = zero-based channel, if present and enabled; no pool-relative remap | Uses channel modulo configured count, then ownership/note checks |
| RoundRobin | Increment cursor, wrap, choose first enabled slot regardless of load | Reverse scan, starting one slot before cursor |
| RoundRobinBalance | Same scan; first inactive slot, else least active, tie by scan order | Same reverse scan |
| Ascending | First inactive in ascending slot order, else least active with lower-ID tie | First matching active slot ascending |
| Descending | First inactive descending, else least active with higher-ID tie | First matching active slot descending |

Cursor starts at 0 and increments before selection; with four enabled idle slots, first RoundRobin Note On goes to slot 1, not slot 0. In balancing, the cursor continues through the scan and is not necessarily the chosen least-loaded slot.

Note Off stops one matching slot, requiring enabled pool membership, same distributor pointer, same channel and active note. It does not release every repeated pitch. Muted distributors ignore incoming Note Off too, but setters attempt to stop owned notes when muting/reconfiguring.

### 8.2 Current state and limits

* Up to 64 distributors, 16 channels and 32 instrument slots.
* Manager owns nonmovable distributors at stable heap addresses.
* Controller tracks only the **last** distributor and channel per instrument slot, not per polyphonic note.
* Configuration changes, strategy replacement and distributor destruction call owned-note cleanup.
* Cleanup uses `stopNote(slot, 0, 0, 0)` as an all-off surrogate. This works for PWM controllers that ignore the note argument, but not Dulcimer, whose note zero is unmapped.
* Removing an index compacts remaining distributor IDs; pointer stability and public index identity are different concepts.

**VOICE-01:** A replacement SHALL track actual active voices with at least logical slot, pitch, channel and stable owner token. True polyphonic engines need ownership per active note/output, not one pointer per slot.

**VOICE-02:** Specify duplicate-note/retrigger and voice-stealing policy. Legacy mono PWM overwrites an active slot; note-overwrite and global polyphony metadata do not prevent it. A new enforced policy SHALL be explicitly selected.

**VOICE-03:** Provide dedicated `stopSlot`, `stopOwned(owner)` and `stopAll` operations. Never encode an all-off request as a musical Note Off for pitch zero.

**VOICE-04:** Configuration mutation/removal SHALL stop affected voices before changing routing and SHALL not leave invalid ownership tokens. Re-fetch indexed distributor state after deletion.

**VOICE-05:** Preserve legacy strategy selection/tie ordering in compatibility mode. Corrected StraightThrough Note Off mapping and improved duplicate-note behavior must be tested as intentional changes.

## 9. Instrument-engine interface

Source: [controller base](../src/Instruments/InstrumentControllerBase.h), [default hooks](../src/Instruments/InstrumentControllerBase.cpp), [singleton selection](../src/Instruments/InstrumentController.h).

### 9.1 Required logical operations

| Operation | Input | Required behavior |
| --- | --- | --- |
| Initialize | Validated engine and hardware profile | Allocate/claim resources and establish safe outputs, or explicit failure |
| Play | Slot, note, velocity, channel, optional owner | Validate, resolve mapping, update ownership only for accepted notes, start output |
| Stop matching note | Slot, note, channel/owner policy | Stop matching voice, not an unrelated replacement note |
| Stop slot | Slot, reason | Stop all slot outputs regardless of pitch |
| Stop owner / all | Owner or global scope | Cancel outputs, pending enables and note state |
| Reset slot / all | Scope | All-off plus engine-specific mechanical/reset policy |
| Active count / note query | Valid slot, optional pitch | Accurate active state, including MIDI note zero |
| Effect update | Channel, controller/effect value | Apply supported effect to matching voices; unsupported is explicit capability |
| Periodic service | Monotonic time | Deferred work, timeout checks, observer flush scheduling |
| Capabilities | No MIDI-specific SDK dependency | Engine type, slots, polyphony, note map, effects, required services |

**ENG-01:** A rejected/mapped-out note SHALL not claim ownership or light a successful-note LED. Bounds are checked before setting external enables, not only inside a base PWM call.

**ENG-02:** Active state SHALL be represented separately from note number and timestamp; pitch 0 and timestamp 0 are valid values.

**ENG-03:** Reset/all-off semantics SHALL be explicit for engines where one "instrument slot" contains multiple actuators. A Dulcimer slot is not equivalent to one mono PWM output.

### 9.2 Current reusable engines and products

| Engine/product | Current behavior to reproduce or deliberately revise |
| --- | --- |
| HwPWM | One note per slot; continuous nominal 50% waveform; note frequency and pitch bend; velocity does not alter duty; stop clears duty and tracking |
| SwPWM | One note per slot; timer-driven toggling; pitch bend, partial modulation/vibrato support; general note timeout |
| StepSw / FloppyDrive | SwPWM-derived step/direction output; simulated unsigned position increment/decrement on **every toggle**, reverse at exact limits; no homing sensor routine |
| StepSwShift / FloppySynth | Same step engine, direction through shift-register outputs; ISR defers direction update to `periodic()` and pauses affected toggles |
| MultiPhase | Per-note timer state advances a configured phase table; current output-index calculation overlaps nominal groups |
| Dulcimer | One logical slot, 40 physical solenoids in chains of 16 + 24; fixed pitch-to-output map; 9 ms profile timeout; retrigger restarts pulse timer; reset of one slot resets all |
| StepperSynthHw | HwPWM plus inverted shift-register enables; five LEDs per slot |
| StepperSynthSw | SwPWM plus inverted shift-register enables; five LEDs per slot; class exists but no named current product environment selects it |
| PneumaticValvesSw | SwPWM with one LED per slot when enabled |
| Custom PwmInstrument / Example | HwPWM with one LED per slot; Example selects this maintained class |

CURRENT generic PWM/MultiPhase timeouts use `elapsed > CFG_NOTE_TIMEOUT_MS`, with zero disabling the timeout. Dulcimer uses `elapsed >= CFG_NOTE_TIMEOUT_MS`; zero would expire on periodic service rather than disable it. `stopAll()` in PWM also recenters channel pitch bends. A replacement SHALL name these policies separately: `maximumSustainMs` versus `pulseOnTimeMs`.

The archived [instrument directory](Old) contains drum, AD9833, Modbus PWM, older floppy and stacked variants. These demonstrate extension needs but do not satisfy the active controller interface. The active percussion header is empty. Reimplement archived capabilities only as separately specified engines/backends; do not claim Stack, SD playback, Modbus or AD9833 is currently working merely because archived code or commented dependencies exist.

## 10. Reusable platform-specific low-level services

This is an architectural requirement, not an instruction to reduce everything to generic `digitalWrite()` calls. Hardware timing and efficient MCU-specific access are central to performance.

### 10.1 Service boundary

**HAL-01:** Shared engines express frequency, duty, pulse timing, output state and safe-stop intent. Backends own timers, PWM modules, GPIO operations, ISR placement, DMA/cache handling and resource constraints.

**HAL-02:** Separate portable musical math/state from hardware execution. A platform backend SHALL not contain MIDI pitch tables, distributor algorithms or product LED mappings.

**HAL-03:** Supply narrow reusable services and optional capability extensions. Compile-time templates/static dispatch are permitted in hard real-time paths; dynamic interfaces need not be called for every waveform edge.

PROPOSED semantic interfaces, independent of programming-language ABI:

| Service | Operations | Explicit units / results |
| --- | --- | --- |
| Monotonic clock | Read time; calculate elapsed | Integer microseconds or wrap-safe milliseconds; width documented |
| Critical section | Enter/restore previous state | Saves prior interrupt/lock state; does not unconditionally enable interrupts |
| GPIO output | Claim/configure; write; masked batch write; safe release | Pin ID, logical level, polarity; validity and ownership result |
| Periodic timer | Claim; configure; attach; start; stop; release | Requested/effective period in microseconds or nanoseconds; priority/resource result |
| Waveform output | Claim; configure/update frequency+duty; start; stop | Frequency in Hz with explicit precision, duty fraction; realized frequency/resolution |
| Step pulse output | Configure pulse width, rate, direction setup/hold; stop | Time units and edge-count semantics; resource/timing result |
| Shift output bank | Set logical bits; latch/update; disable; poll completion | Output count, polarity, chain ordering, completion semantics |
| Persistent storage | Read/write record or snapshot; commit; availability | Byte lengths, schema, integrity and error result |

**HAL-04:** Use an explicit result domain such as invalid pin, unsupported, resource exhausted, frequency unachievable, queue full, hardware failure and integrity failure. Report the requested and realized timing when quantization/clamping occurs.

### 10.2 Platform resource capabilities

A platform capability descriptor SHALL include:

* Exact MCU and board variant, usable output pins and reserved/input-only/boot-sensitive pins.
* PWM channel-to-timer/submodule coupling and maximum **independent frequencies**, not just PWM pin count.
* Timer minimum period, callback capacity, priority options and SDK version.
* Supported duty resolution versus frequency; timer clock source and precision.
* Fast GPIO/batch access, shift/SPI/FlexIO/RMT/DMA support where implemented.
* ISR-safe operations, synchronization model and any flash/cache restrictions.
* Transport/peripheral conflicts, storage capacity and USB capability.

**HAL-05:** Resource allocation SHALL reject incompatible sharing. Updating frequency for one voice SHALL not unexpectedly retune another voice. Identical-frequency sharing is permitted only when explicitly requested.

### 10.3 Current platform implementations

| Function | ESP32 current backend | Teensy 4.1 current backend |
| --- | --- | --- |
| Hardware waveform | LEDC: slot `i` uses channel `2*i`; 12-bit duty 2048; initialize at 1 kHz/off | `analogWriteFrequency`, 8-bit duty 128; initialize at 1 kHz/off |
| Frequency range | Clamp to 10-20000 Hz | Clamp to 10-20000 Hz |
| Frequency update | Reconfigure when cached difference >0.5 Hz | Set frequency on play; no analogous cached update |
| Active pitch bend update | Float result converted to `uint16_t`, apply only if difference >0.75 Hz | Same integer truncation/update threshold |
| Periodic timer | Old Arduino ESP32 timer API: timer 0, prescaler 80; clamps requests below 8 us | `IntervalTimer`; requested profile commonly 1 us |
| Software edge | ISR-marked functions, `digitalWrite` | SwPWM uses `digitalWriteFast`; some step/multiphase paths use `digitalWrite` |
| Shift output | Default bit-bang backend | Teensy fast bit-bang backend selected by current macro comparisons |
| Persistent state | NVS namespace `storage` | Emulated EEPROM |

LEDC's even-channel assignment is intended to avoid shared timers on classic ESP32. Do not generalize "eight independent channels" to every ESP32 variant. Teensy PWM pins can share a hardware submodule; do not assume every configured PWM pin is independently tunable. Hardware frequency/duty feasibility and cross-voice isolation require measurement.

The ESP32 code uses the older `timerBegin(0,80,true)`/LEDC APIs. Pin the supported framework release or implement version-specific adapters; newer SDK signatures are not interchangeable.

### 10.4 Timer ownership, synchronization and ISR rules

CURRENT [InterruptTimer](../src/Instruments/Components/InterruptTimer.cpp) has one global callback. Repeated initialization reuses timer hardware; derived stepping controllers replace their base callback. This is adequate for the currently selected single engine, not multiple independent periodic clients.

**HAL-06:** Define timer ownership explicitly. Either one scheduler owns a timer and invokes registered fixed-capacity jobs, or multiple timer resources are allocated. A new engine SHALL not silently replace another engine's callback.

**RT-01:** ISR work SHALL be bounded and allocation-free. No storage commits, transport calls, serial formatting, LED `show()`, blocking shift transactions or floating-point exponentiation in the waveform ISR. Precompute frequencies/periods and enqueue bounded deferred work.

**RT-02:** Shared ISR/main-loop state SHALL use platform-appropriate atomic operations, snapshot publication or critical sections. On multi-core targets, interrupt masking on one core is not a general cross-core lock. Preserve prior interrupt state for nested calls.

**RT-03:** Deferred direction changes SHALL inhibit relevant pulses until the direction is actually latched and required setup time has elapsed. Latch completion, cancellation on stop, and resumption edge state must be defined.

### 10.5 Note frequency and waveform accuracy

Source: [note tables](../src/Instruments/Components/NoteTable.h).

Nominal equal-tempered frequency:

```text
f(note) = 440 * 2^((note - 69)/12)
bendSemitones = (bend - 8192) * 2 / 8192
fBent = f(note) * 2^(bendSemitones/12)
```

CURRENT hardware PWM uses the stored 128-entry frequency table. Software timing uses a stored full-period table in microseconds, with low-note periods saturated at 65535. It divides by configured resolution (or twice that resolution) using integer division. Tick loops compare the current counter to the target before incrementing, so the edge interval is `(targetTicks + 1) * actualTimerPeriod`, not simply `targetTicks * configuredTimerPeriod`. A toggled square wave has two edge intervals per cycle.

Some profiles define `CFG_PWM_NOTES_DOUBLE` while drivers check `PWM_NOTES_DOUBLE`; this can change output pitch by approximately an octave relative to the intended profile. ESP32 timer clamping can also disagree with table calculations if configured below 8 us. Multiphase pitch additionally depends on steps per table cycle.

For a scratch implementation of the nominal software timing, the stored period table corresponds to microsecond full-cycle periods rounded to the nearest integer and saturated at 65535. Divide that period by the selected full/half-cycle divisor using integer division, then apply the bend multiplier inversely to period and truncate to integer ticks (minimum one). The checked-in explicit table remains the authority for byte-for-byte legacy calculation comparisons; the hardware-frequency table uses six-decimal equal-tempered values.

CURRENT optional vibrato math, when the relevant compile gates actually enable it:

```text
rate = modulationWheel >> 3
phase = (phase + rate) modulo 36000
# Phase update occurs when (currentTick & 7) == 0.
triangle = floor(phase * 254 / 18000) - 127             if phase < 18000
triangle = 127 - floor((phase - 18000) * 254 / 18000)   otherwise
offset = (triangle * depth) arithmetic-shift-right 7
adjustment = (activePeriod * offset) arithmetic-shift-right 10
targetPeriod = clamp(activePeriod - adjustment, 10, 65000)
```

Depth initially comes from modulation-wheel state when device vibrato is enabled; subsequent CC1 updates use the current value on distributor-owned voices. Because phase updates are tied to each voice's tick counter rather than elapsed wall time, this is not a specified fixed-Hz LFO. A reusable replacement modulation engine SHOULD use explicit rate/depth units and a timebase independent of the waveform note, with the legacy algorithm available only as a named compatibility effect.

**RT-04:** Replacement timing SHALL explicitly distinguish full-cycle frequency, edge rate and step rising-edge rate. Generate accurate periods using the realized timer interval and correct rounding; legacy output inaccuracies require an explicit calibration/compatibility option, not silent inheritance.

**RT-05:** Use a wide enough period representation for the product's supported note range. Report unsupported/quantized notes; do not claim 0-127 accurate output merely because an index is in bounds.

### 10.6 Shift-register physical contract

Sources: [bank interface](../src/Instruments/Components/ShiftRegister/IShiftRegister.h), [default backend](../src/Instruments/Components/ShiftRegister/Default_SwShift.cpp), [Teensy backend](../src/Instruments/Components/ShiftRegister/Teensy41_SwShift.cpp).

* Pin argument order: data/SER, clock/CLK, latch/LD, optional enable/EN, optional reset/RST.
* Set/get operates on logical enabled state; output polarity is logical state XOR inversion.
* Both implemented backends actually shift indices from `numOutputs-1` down to 0, despite contradictory LSB-first comments.
* Each bit writes data, raises clock, then lowers clock; after all bits, latch pulses high then low and data is left low.
* `update()` transmits only when dirty; `init()` forces an initial disabled-bank update.
* Default backend uses three 1 us delays per bit and a 1 us latch delay. Teensy uses configured nanosecond delays at corresponding points.
* Optional reset is driven high during initialization. Optional enable is made an output but not explicitly driven to a defined enabled/disabled level.
* Explicit template implementations exist for 8, 12, 16, 24 and 32 outputs. Other widths need implementation/instantiation support.
* The FlexIO header declares operations but its implementation is a placeholder. It is not a working accelerated backend.

**SHIFT-01:** Preserve descending bit transmission for existing wiring and verify the actual chain/Q-output mapping with a walking-bit test. A backend SHALL define EN/RST polarity and safe levels, dirty behavior, inversion changes and completion.

**SHIFT-02:** Acceleration SHALL be reusable for any compatible output bank, not a special case inside Dulcimer or StepperSynth. Implemented capabilities may include fast GPIO, SPI/FlexIO/DMA; unsupported acceleration must fail explicitly.

### 10.7 Motion and multiphase requirements

**MOTION-01:** Motion profiles SHALL define travel units, active step edge, pulse width, direction setup/hold, minimum/maximum positions and whether homing is required. Current position-on-every-toggle logic is not proof of real actuator position.

**MOTION-02:** Enforce limits with ordered/range checks and a defined reset/homing state; avoid unsigned underflow and equality-only reversal. Hardware limit inputs are not implemented by the current `CFG_COMPONENT_LIMITS` flag.

**PHASE-01:** Define each group as a separate enable pin plus an explicit ordered phase-pin list. Index by group stride or per-group arrays. The current `pwmPins[instrument + output + 1]` phase write and `instrument + output` stop indexing overlap groups; do not reproduce that as a correct wiring contract.

**PHASE-02:** Define table dimensions, initial phase, cycle frequency versus update rate and safe disabled pattern. Validate every table element and group pin count.

## 11. Optional add-ons

### 11.1 LED behavior

Sources: [LED state](../src/Extras/AddrLED.h), [LED implementation](../src/Extras/AddrLED.cpp).

CURRENT: FastLED singleton, compile-time strip type/count/order/data pin/brightness. Writes are buffered and marked dirty; `update()` calls `FastLED.show()` once when dirty. No configured updates-per-second scheduling is enforced.

Color behavior:

* Default hue method CHANNEL: `(channel % 4) * 64`.
* Other methods: NOTE `(note % 25) * 10`, PITCH `(note % 128) * 2`, ID `(id % 2) * 125`.
* A shared 8-bit offset increments on each color-generation call and is added to hue by default; this is event progression, not elapsed-time animation.
* Saturation is 255. Brightness uses `velocity * 2` by default (max 254) or 255 when velocity scaling is off.
* Off is black. Range writes are inclusive at both ends.
* Dulcimer maps pitch to LED; StepperSynth maps slot `i` to LEDs `5*i..5*i+4`; PneumaticValves and PwmInstrument use one LED per slot.

Base instrument LED hooks are no-op except setup/update/reset. Enabling LEDs does not make all base engines automatically emit LED events. StepperSynthHw does not currently call LED setup in its constructor; PwmInstrument does not override periodic flushing. These are integration gaps, not desired add-on contracts.

**ADDON-01:** Recommended observer events: accepted note start, note stop, slot reset, global reset, timeout/fault and configuration changed. Include slot, mapped actuator/output, channel, pitch and velocity as needed.

**ADDON-02:** LED mapping/color policy SHALL be separate from physical strip transport. Optional LED setup, event delivery and main-loop flush SHALL be wired consistently for each supported engine.

**ADDON-03:** Add-on work SHALL be bounded, rate-limited and subordinate to actuator timing. Removing an add-on must not change note routing or waveform results. Validate strip lengths and mappings before operation.

### 11.2 SD-card add-on

This is a **specified replacement implementation contract**, not an existing implementation in the reviewed source. Top-level owner command `62` is SD. Storage command `60` remains device/configuration persistence; mounting an SD card must not substitute for NVS/EEPROM persistence.

#### 11.2.1 Reusable service and platform implementations

**SD-01:** The SD add-on SHALL provide mounting/availability, directory enumeration, deterministic file/song indexing, indexed filename lookup, open/read/seek/close, file size and explicit error results. Catalog logic SHALL be shared C++17 code. Filesystem and bus access SHALL be platform backends, reusable by MidiPlayer and other applications without device/distributor dependencies.

| Backend | Required implementation | Hardware Configuration |
| --- | --- | --- |
| Teensy 4.1 native SD | Use the board's built-in SD slot and native SDIO support, through the framework's built-in-card selector (`BUILTIN_SDCARD`) or equivalent SDIO-capable filesystem adapter | Select native SDIO; no guessed external SPI/CS mapping |
| ESP32 external SD reader | Use an external **SPI-connected** SD reader, initialized on the explicitly selected ESP32 SPI bus | Specify SCK, MISO, MOSI, CS, bus identity, validated bus frequency and power/card-detect options |

The ESP32 implementation is not assumed to use native SDMMC, and classic `esp32dev` does not acquire an SD socket by enabling the flag. A different ESP32 SDMMC backend can be added later behind the same service contract. Filesystem/library versions and supported FAT variants SHALL be pinned/documented by the platform profile; unsupported formatting produces an explicit mount error.

**SD-02:** SD/card I/O SHALL run outside waveform ISRs. Enumeration, duration scans and seeks SHALL be incremental or bounded main-loop jobs, with declared per-service budget and command deadline. SPI arbitration SHALL prevent simultaneous transfers by other clients; SD writes/reads must not change another device's chip-select or timer state. Reject pin/resource conflicts through section 3.3.

**SD-03:** Open file handles SHALL have explicit ownership, bounded count and RAII/equivalent close behavior. Read and seek return actual byte count/position or explicit failure; end-of-file is distinct from I/O error. A stale handle/catalog generation SHALL fail rather than select another file.

#### 11.2.2 File and song catalogs

**SD-04:** The platform/product profile SHALL supply the catalog directory; default is the card root. This revision enumerates **regular files in that directory only**, excluding directories, volume entries and filesystem metadata. Enumeration is non-recursive. File names are leaf names relative to that directory, not arbitrary host-supplied paths.

* File indices are contiguous `1..fileCount`, maximum 16382. Index 0 is **not** a file selector and `16383` is reserved.
* Sort by exact filename UTF-8 byte sequence, ascending, case-sensitive. Do not depend on filesystem directory iteration order. Unsupported/invalid filename encodings produce an explicit catalog error, not lossy replacement names.
* Build a separate contiguous song catalog `1..songCount`, maximum 16382, from files with case-insensitive `.mid`/`.midi` extension and a supported SMF header (format 0/1, PPQN division). Order it by the corresponding file order.
* Song index and file index are different namespaces. A non-song file consumes a file index but not a song index. Store `songIndex -> fileIndex`; the player SHALL use that mapping, never assume numerical equality.
* Header-qualified songs may still fail full event/tempo validation; selection reports unsupported/malformed file explicitly. Do not silently skip a failed song during playback.
* If enumeration exceeds capacity, fail catalog publication with CapacityExceeded. Do not silently truncate the card's contents and report a complete catalog.
* Publish a complete catalog atomically and give it a nonzero 28-bit generation, `1..0x0FFFFFFF`. Change generation on successful rescan, unmount/remount or invalidation; zero means no valid published catalog. Do not reuse a live generation after exhaustion: require a new session. Indices are stable **only within one generation**.

**SD-05:** Host reads specify generation to detect card changes. A successful rescan invalidates previous indices/handles and puts a stopped player at the new first song (or index 0 if empty). Rescan while Playing or Paused returns Busy without changing state. Card removal/read failure at any time invalidates the catalog, closes handles, stops and releases player-owned outputs, and exposes a fault. Never fall back to another card/file or acknowledge a missing card as an empty mounted card.

#### 11.2.3 Shared SD/player subcommand envelope and numbers

The owner-profile add-ons keep the section 6 envelope and 120-byte payload limit:

```text
Request:  F0 7D SrcAdr0 SrcAdr1 DestAdr0 DestAdr1 ADDON SUB ARGS... F7
Response: F0 7D DevAdr0 DevAdr1 SrcAdr0 SrcAdr1 ADDON SUB STATUS DATA... F7
# ADDON is 62 (SD) or 63 (MidiPlayer).
```

**ADDON-WIRE-01:** Every validly addressed complete `62`/`63` request SHALL receive exactly one terminal response on the originating transport. Response repeats add-on and subcommand; queries/setters return the schemas below. Long operations may defer their response while serviced incrementally, but must finish or fail within the declared command deadline. There is one outstanding add-on request per host/device session; no new transaction field is inserted. Hosts serialize requests and must not reflect replies as commands.

Explicit Stop/global panic is the cancellation exception to serialization: accept it while preparation is pending. Return the original operation's terminal cancellation response before the Stop response on that ingress, each with its own subcommand. A local/global panic without a requesting transport still cancels the pending operation and returns its error to the original requester.

Do not blindly retry Play or NavigateMs after an ambiguous transport timeout: they can restart a song or apply movement twice. Query SongIndex/State/Pos before reconciling. No exactly-once guarantee is possible without a separately versioned transaction protocol.

| STATUS | Meaning |
| --- | --- |
| `00` | Success |
| `01` | Unsupported add-on/subcommand or unsupported file/feature |
| `02` | Invalid payload length/encoding or boolean |
| `03` | Invalid index/range |
| `04` | SD unavailable/unmounted |
| `05` | Stale catalog generation/invalidated handle |
| `06` | Busy (including rescan while Playing/Paused) |
| `07` | Card/filesystem I/O failure |
| `08` | Malformed song/file |
| `09` | No selected/available song |
| `0A` | CapacityExceeded or duration/value exceeds supported range |
| `0B` | Command deadline exceeded |

Failure responses contain `SUB STATUS` and **no success data**. Errors are also recorded in local diagnostics with operation/card/file context; a failed request must not produce a success-shaped zero value. Unknown subcommand returns `01`; malformed request without a subcommand uses response subcommand `7F` with status `02` **inside** `62`/`63`, not top-level `7F`. Disabled add-ons respond `01` when these dispatcher endpoints are available in the replacement.

**ADDON-WIRE-02:** All fields are big-endian 7-bit groups:

| Type | Bytes | Encoding |
| --- | --- | --- |
| `Index14` / name byte offset/length | 2 | `[(v >> 7) & 7F, v & 7F]`; selector limits as specified |
| `Generation28` | 4 | Shifts 21, 14, 7, 0; each `& 7F` |
| `Ms32` | 5 | Shifts 28, 21, 14, 7, 0; first byte **0..0F**, remaining 0..7F |
| `Bool` / state / status / subcommand | 1 | Bool exactly 0/1; enumerations as defined |
| Raw UTF-8 byte encoded for this wire | 2 per byte | `[(byte >> 7) & 01, byte & 7F]` |

Milliseconds are unsigned integers `0..4294967295` (about 49.7 days). Five 7-bit groups provide 35 bit positions, but unused upper three bits SHALL be zero; reject out-of-range representations rather than wrap. NavigateMs has a separate sign byte and Ms32 magnitude. Native integer endianness and 8-bit high/low address halves must not leak onto the wire.

#### 11.2.4 SD subcommands

All request argument lengths below are exact. Names may exceed one packet; the host requests consecutive chunks.

| SUB | Operation | Request args | Successful response DATA |
| --- | --- | --- | --- |
| `00` | GetInfo | None | `mountState(1), generation(4), fileCount(2), songCount(2)` |
| `01` | Rescan | None | Same as GetInfo, only after complete successful catalog publication |
| `02` | GetFileName | `generation(4), fileIndex(2), byteOffset(2), maxChunkBytes(1)` | `generation(4), fileIndex(2), totalNameBytes(2), byteOffset(2), chunkBytes(1), encodedUtf8BytePairs(2*chunkBytes)` |
| `03` | GetSongFileIndex | `generation(4), songIndex(2)` | `generation(4), songIndex(2), fileIndex(2)` |
| `04`-`7F` | Unassigned SD subcommands | No defined args | Unsupported |

Mount states: `0=Unmounted`, `1=Ready`, `2=Indexing`, `3=Fault`. GetInfo is successful even in those non-ready states because its returned state is the requested information; counts/generation are zero when no valid catalog exists. Other queries require a Ready catalog and an exact generation match. Rescan is not an implicit card-format or recovery/erase command.

**SD-WIRE-01:** GetFileName accepts `maxChunkBytes=1..53` and `byteOffset=0..totalNameBytes`. At the exact end it succeeds with `chunkBytes=0`; past the end is InvalidIndex/Range. Return `min(requestedLimit, remainingBytes)` raw UTF-8 bytes, each encoded as two 7-bit bytes. Name length is measured in UTF-8 bytes, without a NUL terminator. Maximum name length is 16383 bytes; longer names produce CapacityExceeded.

The largest response is `SUB(1)+STATUS(1)+generation(4)+index(2)+length(2)+offset(2)+count(1)+53*2 = 119` payload bytes, or 127 total frame bytes. Decode byte pairs, assemble by offset, then decode UTF-8; chunks may split a UTF-8 code point. Do not NUL-pad or silently shorten a filename. Repeated same-generation requests return identical bytes.

**SD-WIRE-02:** A host first reads GetInfo, then queries names by file index/generation. To display the currently selected song name, query MidiPlayer SongIndex, use GetSongFileIndex, then GetFileName. On StaleGeneration, discard partial names and refresh GetInfo; do not concatenate chunks from different cards/catalogs.

**SD-WIRE-03:** Play and SongIndex SET retain their specified two-byte selector and have no per-request generation field. For explicit host selections, bind the session to the catalog generation returned by its last successful GetInfo/Rescan. If that generation is no longer current, reject the selection with StaleGeneration until the host refreshes GetInfo; never resolve an old host index to a newly inserted card's song. This session binding is adapter/session context, not a field added to Play. Internal EOF/Prev/Next use the player's current validated catalog handle. Card invalidation terminates that binding and playback.

### 11.3 MidiPlayer add-on

Top-level owner command `63` is Playback. MidiPlayer is an optional **producer of normalized musical events**, not a second instrument implementation or an SD bus driver.

#### 11.3.1 Composition, file format and timing

```text
SD platform backend -> shared SD catalog/read/seek service
                    -> SMF decoder + tempo timeline + MidiPlayer state
                    -> normalized events with player origin/owner
                    -> existing MIDI dispatch/distribution/instruments
```

**PLAYER-01:** MidiPlayer SHALL depend on the SD service from section 11.2 and use its indexed handles. Parsing/control/timing logic SHALL be reusable C++17 code shared by Teensy native SD and ESP32 external SPI SD. No filesystem SDK, GPIO or instrument-specific LED code belongs in the player.

**PLAYER-02:** Required input is Standard MIDI File (SMF) format 0 and 1 with PPQN timing and variable-length delta events; format 2 and SMPTE timing are explicitly Unsupported in this revision. Validate `MThd`/`MTrk` lengths, track count, data/status ranges, running status, maximum four-byte VLQs, tempo values and End-of-Track. Truncation is MalformedFile, not normal completion.

* Default tempo is 500000 microseconds/quarter note until a tempo event changes it.
* Merge track events by absolute tick. At equal ticks process tempo metadata first, then other events by track index and original event order. Maintain each track's parser/running-status state separately.
* Convert ticks to elapsed time with checked 64-bit arithmetic and carried fractional precision. Tempo changes affect later tick intervals; use a monotonic playback clock, not loop count or GPIO timer ticks.
* Song duration is the absolute time of the final track End-of-Track after applying the tempo map, including delta time before EOT. Return integer milliseconds rounded down consistently. Reject zero-duration songs as unsupported to avoid an unbounded auto-repeat loop; reject duration above Ms32 capacity.
* Dispatch supported channel Note On/Off, pressure, CC, program and pitch bend through the existing normalized event path. Tempo/meta events control the timeline, not physical outputs.
* File SysEx events SHALL NOT execute MMM configuration/direct-control commands. Skip with a visible diagnostic counter in this revision; unimplemented file events must not become arbitrary device commands. File playback does not automatically forward events to external transports.

**PLAYER-03:** Duration/tempo-map preparation and seeking SHALL be bounded incremental SD jobs. Use bounded read-ahead and sparse checkpoints per profile, not an unbounded all-events-in-RAM parse. Allow only one outstanding selection/seek operation; conflicting operations return Busy. GetState remains one of the three specified states (Stopped while preparing); expose preparation through Busy and diagnostics, not an undocumented fourth wire state.

Validate indices/arguments before mutating state. A preparation operation releases player outputs, freezes at the prior position and retains the prior song selection until the new file is fully prepared. Success publishes the new selection/position and responds; malformed/I/O/deadline failure leaves playback Stopped, does not start another song, and returns the error. Queries requiring unfinished metadata return Busy. Stop/panic can cancel preparation, with the original pending operation receiving an explicit cancellation-as-Busy response; never send two terminal replies.

**PLAYER-04:** Prepare a file's tempo map/duration before starting it. GetMaxPosMs SHALL not guess from file size. Schedule due events in bounded batches and declare latency/late-event limits. If SD read-ahead cannot meet the profile budget, stop player-owned voices and report an underrun/I/O fault rather than sustain a stuck note or emit an unbounded catch-up burst.

#### 11.3.2 Song selection, indices, states and ownership

* Song selection range is `1..16382` (`00 01..7F 7E` on the wire), subject to songCount. `0` means **the current song** for commands accepting that selector; `16383` (`7F 7F`) is reserved and rejected.
* On a Ready nonempty catalog initialize to song 1, position 0, Stopped. On an empty/unavailable catalog there is no selection: returned SongIndex is 0. A zero selector cannot select a nonexistent current song.
* States are exactly `0=Stopped`, `1=Playing`, `2=Paused`.
* Flags default to Repeat=false, AutoPlay=false, Loop=false. Reset preserves these flags; persistence of them is not implicit in command `60` or existing schema 2.
* A command's successful selection response reports the **resolved** actual song index, never the zero-selector alias.

**PLAYER-05:** Identify player-origin notes/voices separately from live network traffic. Pause, Stop, seek, selection, EOF, read fault and card removal SHALL release **all player-owned voices and deferred enables**, but SHALL NOT issue global all-off that silences unrelated live notes. Routing ownership must include origin plus distributor as necessary; a distributor pointer alone is insufficient.

Channel pressure/CC/program/bend state also requires player-source ownership/isolation or explicitly exclusive playback channels; do not erase/recenter a live source's channel state when stopping the player. A profile unable to isolate required effects SHALL reject concurrent conflicting use explicitly. This does not relax an explicit user global panic, which still silences everything.

In particular, file CC120/CC123 SHALL stop only player-owned voices through an origin-aware dispatch hook; passing them unchanged into the current global `stopAll()` handler would violate this contract. Preserve reuse of the core routing/allocation logic, but carry origin through it and distinguish file channel-mode events from an explicit live/global panic.

**PLAYER-06:** Pause stores timeline position and releases active outputs. Resume/seek starts dispatch from upcoming events **without re-triggering notes that began before the saved position**. Maintain a suppressed-note ledger so later Note Offs for skipped pre-position notes cannot stop a newly started player voice or a live voice. Reconstruct program/controller/bend state silently within player scope as needed, not by replaying historical actuator events.

#### 11.3.3 Subcommand dictionary

Successful action DATA is `Selection = songIndex(Index14), state(1), posMs(Ms32)` (8 bytes), unless a row specifies another shape. Use section 11.2.3's `SUB STATUS` prefix and error rules. Exact argument lengths distinguish get/set.

| SUB | Operation | Request args | Successful DATA / behavior |
| --- | --- | --- | --- |
| `00` | Play | `songIndex(Index14)` | Selection; zero resumes saved/current position, nonzero always selects that song and starts at 0 |
| `01` | Pause | None | Selection; Playing -> Paused at saved position; otherwise no-op |
| `02` | Stop | None | Selection; release outputs, Stopped, position 0; retain song |
| `03` | Reset | None | Selection; stop, position 0, select first song; empty catalog -> index 0 |
| `04` | Prev | None | Selection; stop and select previous at position 0; never starts automatically |
| `05` | Next | None | Selection; stop and select next at position 0; never starts automatically |
| `06` | Repeat (Get/Set) | None = GET; `Bool` = SET | One Bool; true selects current again at EOF, false advances |
| `07` | AutoPlay (Get/Set) | None = GET; `Bool` = SET | One Bool; determines whether EOF's selected song starts |
| `08` | Loop (Get/Set) | None = GET; `Bool` = SET | One Bool; allows last-to-first and Prev first-to-last wrapping |
| `09` | GetMaxPosMs | None | `Ms32` duration of selected song |
| `0A` | GetPosMs | None | `Ms32` current song timeline position |
| `0B` | SetPosMs | `Ms32` absolute requested position | Selection; seek/clamp in current song |
| `0C` | SongIndex (Get/Set) | None = GET; `Index14` = SET | GET: actual Index14 (0 if none); SET: Selection, stop and select explicit song at 0; SET zero retains current selection/state/position |
| `0D` | GetState | None | One state byte, 0/1/2 |
| `0E` | NavigateMs | `sign(1), magnitude(Ms32)` | Selection; sign `0=forward`, `1=backward`; relative seek/clamp |
| `0F`-`7F` | Unassigned player subcommands | No defined args | Unsupported |

Changing a flag does not immediately seek, select or change Playing/Paused/Stopped. Bool is not a bit mask: values other than 0/1 are InvalidPayload. Invalid index/magnitude encoding and unsupported subcommand return explicit error without silently interpreting a default.

Empty catalog: GetState/flags/SongIndex and Reset succeed (state 0/index 0); Play, Pause, Stop, Prev, Next, GetMaxPosMs, GetPosMs, SetPosMs, NavigateMs and SongIndex SET require a selected song and return NoSong, or SDUnavailable when unmounted. Reset is not a card mount, rescan or filesystem erase.

#### 11.3.4 Play, Pause, Stop and explicit navigation

| Initial condition / action | Result |
| --- | --- |
| Play `index=0`, Paused | Resume from saved position, Playing |
| Play `index=0`, Playing | No restart; continue current song |
| Play `index=0`, Stopped | Start at current position; normally 0 after Stop/Reset/selection; a stopped seek can set another start position |
| Play `index!=0`, any state | Stop old player voices, prepare/select requested song, start from 0 even if it equals current index |
| Play `index=0`, stopped position at duration | Restart current song from 0 |
| Pause while Playing | Freeze at processed timeline position, release voices, Paused |
| Pause while Paused/Stopped | No-op; preserve position/state |
| Stop | Stopped, position 0, same index |
| Reset | Stopped, position 0, index 1 if available else 0; preserve flags |
| Prev/Next, target exists | Stopped, target index, position 0 regardless of AutoPlay |
| Prev at first / Next at last, Loop=false | Stopped, retain current boundary index, position 0; successful no-wrap navigation |
| Prev at first / Next at last, Loop=true | Stopped, wrap to last/first respectively, position 0 |

A one-song catalog wraps to itself only when Loop permits it; explicit navigation still stops and resets position. Nonzero explicit selectors do not wrap or clamp indices: out-of-catalog and reserved selectors are InvalidIndex.

#### 11.3.5 Song completion and flag precedence

Natural EOF occurs only after all tracks reach a valid EOT and all due events through that position have been processed. Release player voices first. **Selection policy is evaluated before AutoPlay**:

| Repeat | Current index | Loop | Next selection |
| --- | --- | --- | --- |
| true | Any | Either | Same song |
| false | Below last | Either | Current + 1 |
| false | Last | true | First song |
| false | Last | false | No successor; keep last selected at its duration, Stopped |

If a next selection exists, prepare it at position 0. AutoPlay=true starts it; AutoPlay=false leaves it Stopped at 0. Repeat=true/AutoPlay=false therefore resets the current song but does not immediately replay it. Repeat has precedence over Loop. AutoPlay never overrides the terminal no-successor case.

At a terminal last song, Play zero restarts that song; Loop=false does not make explicit Play invalid. Flags do not cause Prev/Next or SongIndex SET to start automatically. I/O errors, malformed files, SD removal, explicit seeks/stops and panic are **not EOF** and must not trigger Repeat/AutoPlay/Loop transitions.

#### 11.3.6 Position control and relative navigation

**PLAYER-07:** GetPosMs and GetMaxPosMs use the current song timeline, not wall-clock elapsed time including pauses or the concatenation of songs. During Playing, position advances on the monotonic playback clock; during Paused/Stopped it is stable.

* SetPosMs clamps a well-encoded request to `[0, durationMs]`.
* NavigateMs computes `currentPosition +/- magnitude` using checked/wide signed arithmetic, then clamps to the same range. Zero magnitude is a no-op.
* Neither seek crosses into another song, even with Loop/Repeat enabled.
* Seek releases player outputs, uses checkpoints/tempo map, silently reconstructs player channel state, and positions at the first event at or after the target that has not already been consumed at an unchanged position. Seeking to a different position permits replay of events at that target, but does not re-create earlier held notes.
* Seeking within `[0,duration)` preserves prior state: Playing resumes there, Paused remains paused, Stopped remains stopped with that position available to Play zero.
* Seeking to duration leaves the player Stopped at duration, with no EOF auto-transition. This rule applies from Playing or Paused too.
* A successful seek response is sent only when the position is ready. While preparing, processing budgets and Busy behavior from PLAYER-03 apply.

Internally retain sub-millisecond timing precision for scheduling. The reported endpoint is the rounded-down duration; an explicit seek to that advertised endpoint is deliberately treated as terminal, even if the precise EOT is fractionally later. A no-op seek must not re-dispatch same-position events.

No audio-style reverse playback is implemented by negative NavigateMs; it is a backward timeline seek. No one-byte signed value or two-byte millisecond limit is assumed.

#### 11.3.7 Wire examples

Hex examples use device `00 18` and server `7F 7F`. All successful responses below are examples, not observed firmware captures.

```text
Get SD info:
F0 7D 7F 7F 00 18 62 00 F7
# Ready, generation 1, two files, one song:
F0 7D 00 18 7F 7F 62 00 00 01 00 00 00 01 00 02 00 01 F7

Get first filename, generation 1, offset 0, up to 53 raw UTF-8 bytes:
F0 7D 7F 7F 00 18 62 02 00 00 00 01 00 01 00 00 35 F7
# "a.mid", total 5 bytes, returned as five high/low pairs:
F0 7D 00 18 7F 7F 62 02 00 00 00 00 01 00 01 00 05 00 00 05 00 61 00 2E 00 6D 00 69 00 64 F7

Play explicit song 1 from its start:
F0 7D 7F 7F 00 18 63 00 00 01 F7
# Success: actual song 1, Playing, position 0:
F0 7D 00 18 7F 7F 63 00 00 00 01 01 00 00 00 00 00 F7

Play current song (resume semantics):
F0 7D 7F 7F 00 18 63 00 00 00 F7

Pause:
F0 7D 7F 7F 00 18 63 01 F7

Set Repeat true:
F0 7D 7F 7F 00 18 63 06 01 F7
# Success, returned flag true:
F0 7D 00 18 7F 7F 63 06 00 01 F7

Get position:
F0 7D 7F 7F 00 18 63 0A F7
# Success, 1000 ms = 00 00 00 07 68:
F0 7D 00 18 7F 7F 63 0A 00 00 00 00 07 68 F7

Set position to 1000 ms:
F0 7D 7F 7F 00 18 63 0B 00 00 00 07 68 F7

Navigate backward 1000 ms:
F0 7D 7F 7F 00 18 63 0E 01 00 00 00 07 68 F7

Play reserved index 16383:
F0 7D 7F 7F 00 18 63 00 7F 7F F7
# InvalidIndex, no success body and no top-level Error command:
F0 7D 00 18 7F 7F 63 00 03 F7
```

### 11.4 Other add-ons and interoperability

LED and SD are independent add-ons; MidiPlayer depends on SD but not LEDs. Optional playback indicators observe player events rather than modifying its state machine. Diagnostics share the existing bounded service/notification architecture.

The original source ignores `62`/`63` and cannot be assumed to acknowledge these subcommands. Enable the owner add-on protocol only for a peer explicitly configured to support this revision. Keep legacy device/distributor constructs unchanged; add-on names, indices, positions and flags are not inserted into their reserved bytes.

## 12. Persistence interface and on-device compatibility

Sources: [storage interface](../src/Extras/LocalStorage/ILocalStorage.h), [factory](../src/Extras/LocalStorage/LocalStorageFactory.h), [ESP32 backend](../src/Extras/LocalStorage/ESP32LocalStorage.cpp), [Teensy backend](../src/Extras/LocalStorage/Teensy41LocalStorage.cpp).

### 12.1 Logical persistence

Stored fields: runtime name, ID, device flags, distributor count and each 24-byte construct. Device polyphony and instrument-type metadata are not persisted. Transport selection, GPIO wiring and engine class remain compiled settings.

CURRENT initialization occurs on first factory access. Setter operations usually write immediately. The null backend reports unavailable and has no persistent writes. There is no common transactional snapshot, export/import wire command or reliable write-success acknowledgment.

### 12.2 Schema 2 and checksum

Distributor record = 24-byte legacy construct + one **raw 8-bit** CRC byte. The CRC is not part of SysEx and need not be 7-bit.

```text
CRC-8: polynomial 0x07, initial value 0x00, no reflection, final XOR 0x00.
For each byte:
    crc ^= byte
    repeat 8 times:
        crc = ((crc << 1) XOR 07) if old top bit set, else (crc << 1)
        retain low 8 bits
```

Device metadata has no equivalent record CRC. Corrupt distributor records are skipped on load. This can renumber later runtime indices; loading uses normal add methods and writes records/count back during restoration. Power loss can therefore damage the persisted topology.

### 12.3 Teensy EEPROM layout

Source: [EEPROM constants](../src/Extras/LocalStorage/Teensy41LocalStorage.h).

| Address | Bytes | Content |
| --- | --- | --- |
| `0000` | 2 | Magic `A5 B6`, big-endian raw bytes |
| `0002` | 20 | Space-padded device name |
| `0016` | 2 | Device ID, 7-bit high then low |
| `0018` | 2 | Device flags, 7-bit high then low |
| `001A` | 1 | Distributor count, capped at 64 |
| `001B` | 1 | Schema version `02` |
| `001C + 25*i` | 25 each | Distributor `i`: construct + CRC |

64 records occupy `001C..065B` inclusive. Invalid magic/version triggers initialization of metadata/defaults. Reset reinitializes metadata/count but does not erase all old distributor bytes.

### 12.4 ESP32 NVS layout

Namespace: `storage`.

| Key | Type | Value |
| --- | --- | --- |
| `Schema_version` | U8 | `02` |
| `Device_name` | Blob, 20 bytes | Space-padded name |
| `Device_id` | Blob, 2 bytes | ID high/low 7-bit groups |
| `Device_bool` | Blob, 2 bytes | Flags high/low 7-bit groups |
| `Distributor_num` | U8 | Count capped at 64 |
| `Distributor` + four hex digits | Blob, 25 bytes | Construct + CRC |

The distributor key digits are **least-significant nibble first**: index 0 -> `Distributor0000`; 1 -> `Distributor1000`; 16 -> `Distributor0100`; 63 -> `Distributorf300`. Do not use normal `%04x` order when importing existing NVS.

Missing schema version is stamped 2; an explicit other version causes key reset. NVS initialization may erase the whole NVS partition for no-free-pages/new-version errors. This is broader than deleting MMM keys. Reset deletes known metadata plus historical distributor key indices 0-255.

Both backends write names space-padded but wire GET is NUL-padded. Their source attempts to trim spaces/NULs using `find_last_not_of(" \0")`; the C-string overload only supplies the space character, so trailing NUL trimming is not reliable. An all-space name falls back to the current runtime default.

### 12.5 Replacement persistence requirements

* **STORE-01:** Separate read-only restore from mutation/commit. Do not write while enumerating stored configuration.
* **STORE-02:** Validate schema, exact record lengths, CRC, IDs, ranges, capabilities and topology before activation.
* **STORE-03:** Commit a configuration atomically, using backend transactions or a versioned two-slot journal with sequence/integrity and commit marker. Recover the last complete snapshot after interrupted writes.
* **STORE-04:** Report unavailable, missing, corrupt and failed commit distinctly. A runtime change notification does not prove durability.
* **STORE-05:** Do not silently erase unrelated platform data. Migration/reset must define scope, diagnostics and backup/recovery policy.
* **STORE-06:** Preserve schema-2 import when updating existing hardware; a brand-new device interoperating only over MIDI need not emulate EEPROM addresses.
* **STORE-07:** New schemas SHALL have explicit migration rules. Leave legacy wire constructs unchanged even if the internal storage format improves.

## 13. Compatibility decisions and review findings

These are functional interoperability/design findings, not a claim of completed remediation. No firmware or existing configuration was changed to write this document.

| Area | Observed discrepancy | Replacement treatment |
| --- | --- | --- |
| USB SysEx | Transmit omits `F0` in already-framed SDK mode | Fix adapter framing; retain wire schema; test actual host bytes |
| MIDI Start/Stop | `FA` stops; `FC` ignored | Standard behavior by default; explicit legacy behavior policy if needed |
| Device flags | Mute/omni/damper/polyphony largely metadata | Negotiate/select enforcement; do not misreport capabilities |
| Note overwrite | Stored flag not used | Define policy explicitly rather than infer from flag name |
| Direct control | Bypasses distribution and often channel ownership | Explicit origin/voice ownership; gate control while fault-safe |
| Owned-note cleanup | Pitch-zero surrogate fails for Dulcimer | Dedicated stop-owned operation |
| Active state | SwPWM/MultiPhase can report inactive slot as active note 0; Dulcimer uses timestamp 0 sentinel | Independent active flag/voice record |
| Safety timing | Loop-driven timeouts and differing zero semantics | Per-engine timeout policy and independent cutoff where needed |
| Profile wiring | ESP32 Dulcimer GPIO 33 is both chain-2 EN and enabled LED data | Reject profile conflict and verify corrected wiring with user/hardware |
| Multiphase groups | Phase/stop writes overlap group offsets | Explicit group topology; no unsafe defect emulation |
| Feature gates | `CFG_PWM_NOTES_DOUBLE` vs `PWM_NOTES_DOUBLE`; `CFG_VIBRATO_ENABLED` vs `VIBRATO_ENABLED` | One canonical typed setting with compatibility mapping |
| Shift backend | Undefined symbolic-token comparisons; FlexIO body absent | Explicit enum and availability checks |
| Unsupported USB | Inert adapter allows ESP32 builds to appear successful | Availability error, or explicitly remove USB from profile |
| PWM capacity | Protocol capacity and pin count do not establish independent timing | Resource-group validation and cross-voice hardware tests |
| Serial recovery | In SysEx, unexpected status is appended; overrun discard can consume following events | Define status resynchronization and partial-frame deadline |
| Serial boundary | If overflow is discovered on the terminator byte, discard mode remains until another `F7` | Regression-test 128/129-byte boundaries and recovery |
| Numeric fidelity | Low-note saturation, integer period rounding, extra tick, truncated bent frequency | Report realized frequency; measured calibration policy |
| Effects arrays | Modulation-wheel storage sized by slot count but indexed by MIDI channel | Channel state sized to 16 independently of slot count |
| LED integration | Refresh macro unused; some products omit setup/flush/event hooks | Common observer lifecycle and bounded refresh |
| Storage | Restore writes; invalid records compact topology; errors often hidden | Read-only restore, explicit integrity failures, atomic snapshots |
| Owner command map | Source `7F` Error conflicts with owner `7F` Extend; source `62` label conflates SD/playback | Explicit peer profile; separate SD `62` and MidiPlayer `63`, with status in add-on replies |
| C++ platform scope | Existing enums/fallbacks mention Uno/Mega without complete maintained support | C++17 edition for capable MCUs; separate legacy-controller edition and capacity contracts |
| Documentation | Existing reports describe earlier source states and some stale capabilities | This ICD's tables take precedence for this reviewed baseline |

**COMPAT-01:** Wire-compatible corrections and musical-policy changes SHALL be distinguished in release notes and profile metadata. Do not require malformed frames or unsafe GPIO behavior for conformance.

**COMPAT-02:** A host compatibility adapter MAY tolerate a known legacy device defect only on the affected peer/transport, with bounded validation and diagnostics. Do not make every parser accept arbitrary unframed data.

## 14. Proposed evolution and reimplementation sequence

### 14.1 Legacy host procedure

1. Open the selected serial/USB MIDI transport without debug text on the MIDI stream.
2. Discover with `02`; establish a unique device ID if necessary.
3. Read `21` for device metadata.
4. Read `30`; enumerate each distributor with `40` and a 2-byte selector. Do not use `20` or `32` for aggregate data.
5. Stop outputs as necessary before routing changes.
6. Configure with `31`/`40` or individual setters. Do not wait indefinitely for a success ACK that is not defined.
7. On `04`, re-read relevant state. Re-read the full distributor list after deletion because indices compact.
8. Query back after writes to verify runtime state. Persistence still requires reboot/explicit backend verification.
9. Send normal MIDI after channels and pools are configured; use direct commands only with an explicit control policy.

### 14.2 Defined owner add-ons and further protocol evolution

Revision 1.1 defines owner-profile `62` SD and `63` MidiPlayer subcommands in sections 11.2-11.3. Their explicit status prefix and exact field widths are new implementation requirements, not existing-source behavior. The owner's remaining Reserved/Extend slots in section 6.4 are not assigned new meanings here.

Retain the legacy envelope behind a codec/compatibility adapter. Define a separately versioned protocol revision before adding:

* Capability discovery and supported-command/effect/backend reporting.
* Request IDs, explicit ACK/error, configuration revision and persistence status.
* Paginated snapshot transfer with bounded chunks and atomic apply.
* Stable external distributor identity independent of compacted list position.
* Optional integrity and protected management sessions for new network transports.

Do not repurpose reserved bytes in existing constructs without a negotiated extension. A 64-distributor snapshot needs 1536 bytes for constructs alone, far beyond 120 payload bytes. New pagination must define ordering, revision consistency, retries, completion and partial-transfer abort.

No further expansion-point payloads, general transaction IDs or authenticated-session schema are assigned by this revision. These require a separate approved ICD revision and host implementation. Until capability discovery is defined, provision peer support for the owner add-on profile explicitly; legacy clients retain source-baseline compatibility without sending unsupported add-on requests.

### 14.3 Incremental implementation plan

1. Implement legacy codec and command behavior with golden vectors, using an in-memory fake instrument.
2. Implement portable routing and explicit voice ownership; test overlapping pools and cleanup.
3. Consolidate PlatformIO settings and validate resolved profiles while keeping old environment aliases.
4. Implement reusable platform clock/GPIO/timer/PWM/shift services and verify resource independence.
5. Compose mono waveform, stepping, multiphase and mapped percussion engines.
6. Add LED observers and schema-2 storage import; implement reliable new snapshot storage.
7. Implement the shared SD catalog/service, Teensy native SDIO and ESP32 external SPI backends, and `62` subcommands.
8. Implement SMF timing/seek, player-owned voice isolation, the complete MidiPlayer state/flag machine and `63` subcommands.
9. Validate serial/USB round trips against existing host/device combinations and both add-on backends.
10. Introduce corrected musical policies under explicit profiles/versioned capabilities.
11. Add further transports/add-ons without changing the core interfaces; implement Uno/Mega under the separate legacy edition.

## 15. Verification and acceptance

### 15.1 Evidence available from this review

Revisions 1.0 and 1.1 are documentation-only tasks. No firmware builds, native tests, uploads or hardware measurements were run for this ICD. The [existing native test file](../test/test_protocol/test_main.cpp) contains four tests: complete SysEx construction/address decoding, incomplete framing, payload bounding and CRC corruption. It does **not** verify command dictionaries, distribution, USB framing, storage power-loss behavior, SD backends, MidiPlayer or real-time performance.

Earlier reports record successful builds and unavailable native host compiler tooling, but those statements are historical evidence, not fresh certification of this document's baseline.

### 15.2 Required conformance tests

| Test ID | Interface/requirement | Exact acceptance |
| --- | --- | --- |
| T-WIRE-01 | WIRE-01/02, DATA-01 | Golden request/reply bytes exactly match sections 6-7; all reserved GET bytes zero |
| T-WIRE-02 | Packet capacity | Accept complete 8-byte empty and 128-byte maximum frame; reject/recover from 129+; payload maximum exactly 120 |
| T-WIRE-03 | Integer encoding | Round-trip all 14-bit IDs and each of 16/32 mask bits; include high-bit examples from section 6 |
| T-WIRE-04 | Addressing | Local and broadcast handled; other device/manufacturer ignored; replies target request source |
| T-CMD-01 | Dictionary | Test GET/SET payload thresholds, unsupported commands, ignored commands and exact errors for every command in section 7 |
| T-CMD-02 | Distributor limit | IDs 0/63 accepted, 64/16383 rejected without unbounded allocation; count never exceeds 64 |
| T-CMD-03 | Mutation/notification | Query-back equals accepted state; ID changes use new address; legacy empty replies/no replies honored |
| T-NET-01 | Serial parser | Every possible split of a valid frame, running status, 2/3-byte families and real-time interleaving give identical normalized messages |
| T-NET-02 | Recovery | Unterminated/overlong SysEx, unexpected status and overflow-on-terminator followed by valid notes recover by documented deadline |
| T-NET-03 | USB MIDI | Host-visible send/receive includes exactly one `F0` and `F7`; verify 56/24-byte constructs and 128-byte frame, channel and bend conversions |
| T-NET-04 | Multiple transports | Reply goes to ingress only; notification policy matches mode; no automatic MIDI echo; saturated ingress does not starve another |
| T-MIDI-01 | MIDI-01/02/04 | Note-on zero, CC120/123, FA/FC/FF and pitch bend endpoints/center behave as explicitly selected policy |
| T-VOICE-01 | VOICE-01..05 | Exact allocation/tie/cursor sequence, repeated pitches, overlapping pools, retriggers and owner-matched release |
| T-VOICE-02 | All-off | Reconfigure/remove/mute/panic leaves zero physical enables, zero active voices and no pending re-enable, including Dulcimer and note 0 |
| T-ENG-01 | Bounds/maps | All slots/notes/channels boundary-tested; rejected mapped-out notes produce no output/LED/ownership |
| T-ENG-02 | Timeout | Check just before, at and after deadline, zero policy, timestamp zero, unsigned rollover and retrigger |
| T-CFG-01 | CFG-01..07, CPP-01..03 | Validate all five shared sections and per-platform Main/Hardware composition; build the 26 existing C++17 aliases; fail missing SD/player dependencies, duplicate pins and unsupported backends/resources; do not claim Uno/Mega support in this edition |
| T-HAL-01 | HAL-04/05 | Frequencies/duty realized within declared profile tolerances; retuning one voice does not alter another |
| T-HAL-02 | HAL-06, RT-01/02 | Timer claims and callback ownership enforced; concurrency stress has no torn state, unsafe nested unlock or ISR blocking work |
| T-SHIFT-01 | SHIFT-01/02 | Walking one-bit test across complete chains, inversion and EN/RST safe states; dirty update and hardware completion verified |
| T-MOTION-01 | MOTION/PHASE | Direction setup/hold and pulse widths meet external driver requirements; limits/homing/phase-group isolation tested |
| T-LED-01 | ADDON-01..03 | Correct mappings/color/on-off; setup/flush works; no out-of-range write; disabled add-on leaves audio/motion behavior unchanged |
| T-STORE-01 | Schema import | Exact EEPROM/NVS layouts and CRC reproduced; malformed length/schema/CRC explicitly diagnosed |
| T-STORE-02 | STORE-01..07 | Read-only restore writes nothing; injected power loss at each commit stage recovers last complete topology; unrelated NVS keys preserved |
| T-OWNER-01 | OWNER-01/02 | Cover every top-level byte `00..7F`, including all Reserved/Extend and custom widths; verify server pair `7F 7F` decodes to `3FFF`; never confuse source Error with owner Extend |
| T-ADDON-WIRE-01 | ADDON-WIRE-01/02 | Exact get/set/action/status lengths; 5-byte Ms32 round-trips at 0/127/128/1000/4294967295; reject nonzero unused top bits, invalid bool/sign, unknown subcommands and timeouts without success data |
| T-SD-01 | SD-01..05 | Same deterministic file/song catalogs on Teensy native SDIO and ESP32 external SPI; non-song file gaps affect only file indices; empty/malformed/oversized catalogs and stale handles explicitly fail |
| T-SD-02 | SD-WIRE-01..03 | Filenames at indices 1/16382, reject 0/16383/out-of-count; exact 53-byte chunk limit; offsets at/past end; UTF-8 code point across chunks; 127-byte max response; generation changes reject partial name assembly and old Play selectors |
| T-SD-03 | SD-02/05 | Pin/bus arbitration and main-loop bounds; rescan Busy while Playing/Paused; card removal during read/seek releases only player-owned outputs, closes handles and reports fault |
| T-PLAYER-01 | PLAYER-01..04 | SMF 0/1 with multitrack merge, independent running status, tempo changes/default, simultaneous events and delayed EOT; exact derived duration; format2/SMPTE/zero-duration/truncated/VLQ/oversized duration failures |
| T-PLAYER-02 | Commands `00..0E` | Exercise every player GET/SET/action with exact request/reply schemas; Play zero versus explicit same/different index; limits 1/16382 and empty catalog; invalid index/payload causes no silent fallback |
| T-PLAYER-03 | Completion precedence | All 8 Repeat/AutoPlay/Loop combinations at first/middle/last song and one-song catalog; terminal EOF retains last at duration; selection errors stop explicitly rather than skip |
| T-PLAYER-04 | PLAYER-05/06 | Pause/Stop/Reset/seek/EOF release player voices and pending work, not concurrent live voices/effects; no pre-position note retrigger; suppressed Note Off cannot stop a new/live voice |
| T-PLAYER-05 | PLAYER-07 | Seek from each state, clamp below/above range, sign+Ms32 navigation without overflow, 0 movement, no-op same-position event suppression, seek-to-duration without auto-transition |
| T-PLAYER-06 | Navigation/state | Prev/Next always Stopped at 0; Loop wraps both ends, otherwise retains boundary; Reset selects first/preserves flags; GetState never returns an undocumented loading state |
| T-PLAYER-07 | Preparation/faults | Incremental scans/seeks obey budgets/deadlines; conflicting operation Busy; Stop cancels with one terminal response; SD underrun, removal, failed parse and embedded MMM SysEx never cause unwanted actuation or EOF advance |
| T-SYS-01 | LIFE-01..03 | Startup/fault/reconnect/reconfiguration/all-off leave safe outputs; blocked loop triggers independent cutoff when required |

### 15.3 Performance acceptance must be measured

The repository supplies no measured jitter, maximum latency or thermal limits. They must not be invented as existing guarantees.

**PERF-01:** Each released board/product profile SHALL declare numeric budgets for:

* Supported simultaneous independent voices and note/frequency range.
* Maximum input-to-output latency (including ingress queue), maximum all-off latency.
* Frequency error in percent or cents, peak edge jitter, duty/pulse-width tolerance.
* Maximum ISR execution time and longest main-loop service interval.
* Maximum output on-time and independent cutoff deadline where applicable.
* LED refresh rate/maximum flush duration and permitted interference.
* SD read-ahead size, maximum filesystem service slice/latency and open-handle/catalog/checkpoint capacities.
* MidiPlayer event-batch limit, scheduling lateness/underrun threshold, scan/seek/command deadlines and memory ceiling.

Use a logic analyzer/oscilloscope while loading all voices, pitch-bending, changing directions, processing both networks and refreshing LEDs. Include storage commits, both SD backends, multitrack playback/seek/rescan, removal/underrun injection and stalled-loop tests.

Acceptance equations:

```text
frequencyErrorCents = 1200 * log2(measuredHz / requestedHz)
maxISRExecutionTime < realizedTimerInterval
measuredLatency <= profileLatencyBudget
measuredPeakJitter <= profileJitterBudget
measuredOutputOnTime <= profileMaximumOnTime + declaredCutoffTolerance
```

Recommended scheduler headroom target: ISR worst-case <=50% of its interval, unless a measured product-specific allocation justifies another bound. This is a **proposed engineering target**, not a current 1 us/8 us performance claim. External driver datasheet timing and verified actuator limits determine physical acceptance.

## 16. Open evidence and source traceability

### 16.1 Required evidence before claiming drop-in interoperability

* Capture the actual GUI's requests/replies and record deployed firmware versions; confirm no undocumented layouts or previously different semantics.
* Verify actual USB SysEx output and framework versions on both old and replacement devices.
* Resolve GPIO conflicts and electrical pin roles against physical wiring, including optional EN/RST and boot-sensitive ESP32 pins.
* Measure PWM pin/timer coupling, realized low/high-note frequencies, all-off latency, ISR occupancy and LED interference.
* Define safe on-time, cooldown/current/thermal limits and motion/homing requirements per real mechanism. GPIO pin maps are not a wiring/power-driver specification.
* Establish supported toolchain/platform/library versions and actual repository-root CI wiring.
* Obtain explicit approval for revised global mute/omni/sustain/polyphony/Start/Stop and voice-stealing policies.
* Verify the owner-protocol versus source-baseline profile with actual hosts, especially `7F` allocation and new `62`/`63` bodies.
* Supply validated external ESP32 SD-reader wiring, filesystem formatting/library versions, memory and I/O timing budgets; test Teensy native SDIO independently.
* Verify file/song index mapping, UTF-8 name chunks, SMF durations and the agreed Repeat/AutoPlay/Loop/seek/pause rules against host UI behavior.

These unresolved items do not prevent reimplementing the source-defined codec and architecture. They do prevent asserting verified hardware safety or universal compatibility with the separately hosted GUI.

### 16.2 Source map

| Contract | Primary source |
| --- | --- |
| System lifecycle | [main](../src/main.cpp) |
| Build, platform detection, defaults | [PlatformIO](../platformio.ini), [Config](../src/Config.h), [profiles](../src/config) |
| IDs/enums | [Constants](../src/Constants.h) |
| Device construct | [Device](../src/Device.h) |
| Packet format | [MidiMessage](../src/MsgHandling/MidiMessage.h), [mask encoding](../src/Utility/BitManipulation.h) |
| Command dictionary | [SysEx handler](../src/MsgHandling/SysExMsgHandler.cpp) |
| Musical dispatch | [MIDI handler](../src/MsgHandling/MidiMsgHandler.cpp) |
| Routing/transport composition | [router](../src/MsgHandling/MessageRouter.cpp), [network manager](../src/Networks/NetworkManager.h) |
| Serial/USB/descriptor behavior | [serial](../src/Networks/NetworkSerial.cpp), [USB](../src/Networks/NetworkUSB/Teensy41_NetworkUSB.cpp), [USB support files](../src/Networks/NetworkUSB) |
| Distribution/ownership | [Distributor](../src/Distributors/Distributor.cpp), [manager](../src/Distributors/DistributorManager.cpp), [strategies](../src/Distributors/DistributionStrategies.cpp) |
| Controller extension | [base contract](../src/Instruments/InstrumentControllerBase.h), [custom example](../src/Instruments/Custom/PwmInstrument.h) |
| Waveform/step/phase implementations | [base engines](../src/Instruments/Base), [composed products](../src/Instruments/DJthefirst) |
| Timer/math/shift | [InterruptTimer](../src/Instruments/Components/InterruptTimer.cpp), [NoteTable](../src/Instruments/Components/NoteTable.h), [shift services](../src/Instruments/Components/ShiftRegister) |
| LED add-on | [AddrLED](../src/Extras/AddrLED.cpp) |
| Persistent schemas | [local storage](../src/Extras/LocalStorage), [CRC](../src/Utility/Crc8.h) |
| Existing verification | [native tests](../test/test_protocol/test_main.cpp), [matrix script](../scripts/test_instrument_compilation.ps1), [nested workflow](../.github/workflows/platformio-matrix.yml) |
| Historical scope only | [networks](../LegacyCode), [archived instruments/configurations](Old) |
| Owner command allocation and requested configuration hierarchy | Owner-supplied protocol/section requirements, 2026-11-11; specified in sections 1.3, 3.3 and 6.4, not inferred from existing source |
| New SD and MidiPlayer contracts | Sections 11.2-11.3; new subcommand assignments and agreed completion/navigation/pause rules; no existing implementation asserted |

## Appendix A. Current board/product profiles

These are configuration values, **not validated electrical designs**. `T` = Teensy 4.1; `E` = ESP32 `esp32dev`. All profiles use 1 subinstrument/group, note metadata 0-127 and serial at 115200. Timer resolution is T=1 us, E=8 us. Counts below are the effective logical slot counts.

### A.1 Identity and enabled features

| Product section / environment prefix | Device name | ID | Slots T/E | Timeout ms | Storage | LEDs enabled | USB selected |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Dulcimer | Teensy Dulcimer | `0018` | 1/1 | 9 pulse | Yes | 115 | Yes; PID `0501` |
| FloppySynth | Teensy FloppySynth | `0018` | 8/8 | 10000 | Yes | No | No |
| StepperSynthHw (base StepperSynth) | StepperSynth2 | `0003` | 10/8 | 10000 | Yes | 50 | Yes; PID `0500` |
| Example | Teensy Example | `0018` | 1/1 | 10000 | Yes | No | No |
| MultiPhase | MultiPhase | `0018` | 2/2 | 10000 | Yes | No | No |
| FloppyDrive | Floppy Drive | `0033` | 8/8 | 10000 | Yes | No | No |
| StepSw | Step SW PWM | `0033` | 8/8 | 10000 | Yes | No | No |
| StepSwShift | StepSwShift | `0018` | 12/8 | 50 | Yes | No | No |
| PwmHw | Hardware PWM | `0018` | 12/8 | 10000 | No | No | No |
| PwmSw | Software PWM | `0018` | 12/8 | 10000 | No | No | No |
| PneumaticValves | Teensy Pneu Valves | `0018` | 8/8 | 10000 | Yes | No | No |
| Teensy41HwPwm | Teensy 4.1 HW PWM | `0041` | 12/8 | 10000 | Yes | No | No |
| Teensy41SwPwm | Teensy 4.1 SW PWM | `0043` | 16/10 | 10000 | Yes | No | No |

Both suffixes `_teensy41` and `_esp32dev` exist for each prefix. USB selected on E means the inert unsupported adapter, not working USB MIDI. Device names can exceed the standalone SET-name 19-byte limit.

### A.2 PWM/step/phase pins in configured order

| Product | T pins | E pins |
| --- | --- | --- |
| Dulcimer | No PWM; see chains below | No PWM; see chains below |
| FloppySynth | 1,2,3,4,5,6,7,8,9,10 (first 8 active slots) | 4,13,14,19,21,22,23,25 |
| StepperSynthHw | 23,22,28,6,7,24,3,4,5,36 | 33,32,19,21,22,23,2,4 |
| Example | 6 | 23 |
| MultiPhase | 1,2,3,4,5,6,7,8 | 2,4,18,19,21,22,23,33 |
| FloppyDrive / StepSw | 0,2,4,6,8,10,14,18 | 4,13,14,16,17,18,19,21 |
| StepSwShift | 1,2,3,4,5,6,7,8,9,10,11,12 | 4,13,14,19,21,22,23,25 |
| PwmHw | 2,3,4,5,6,8,9,10,14,18,22,28 | 2,4,18,19,21,22,23,25 |
| PwmSw | 1,2,3,4,5,6,7,8,9,10,11,12 | 2,4,18,19,21,22,23,25 |
| PneumaticValves | 1,4,3,2,5,6,7,8 | 4,13,14,16,17,18,19,21 |
| Teensy41HwPwm | 2,3,4,5,6,8,9,10,14,18,22,28 | 4,13,14,16,17,18,19,21 |
| Teensy41SwPwm | 2,3,4,5,6,7,8,9,10,11,12,14,15,16,17,18 | 4,13,14,16,17,18,19,21,22,23 |

MultiPhase configuration intends two groups of enable + three phase outputs, table `{100,010,001}` with three steps. Current indexing does not implement the intended group stride; section 10.7 controls the corrected interface.

### A.3 Direction, shift chains and LED wiring

| Product/component | T | E |
| --- | --- | --- |
| FloppyDrive / StepSw direction pins | 1,3,5,7,9,11,15,19 | 5,15,22,23,25,26,27,32 |
| FloppySynth shift data/clock/latch | 11,12,13; 16 outputs | 16,17,18; 16 outputs |
| StepSwShift shift data/clock/latch | 13,14,15; 16 outputs | 16,17,18; 8 outputs |
| StepperSynthHw shift data/clock/latch | 17,16,15; 16 outputs; inverted | 25,27,26; 16 outputs; inverted |
| Dulcimer chain 1 data/clock/latch/EN/RST | 1,4,3,2,5; 16 outputs | 2,4,18,19,21; 16 outputs |
| Dulcimer chain 2 data/clock/latch/EN/RST | 8,11,10,9,12; 24 outputs | 22,23,32,33,25; 24 outputs |
| Enabled Dulcimer LED data | 29 | 33 (**conflicts with chain-2 EN**) |
| Enabled StepperSynthHw LED data | 13 | 18 |

Dulcimer uses 30 ns hold configuration, other configured shift profiles use 25 ns; actual default E shifting uses microsecond delays. Current enabled LEDs use WS2811, GRB, brightness 255. Other profiles contain dormant LED parameters/pins; enabling them requires validating complete wiring and lifecycle, not simply uncommenting an extra.

FloppyDrive/FloppySynth use limits 0/150. StepSw has commented limit flags but still receives the defaults 0/150 in its stepping code. StepSwShift likewise uses defaults. These are software counts, not verified homing/travel measurements.

## Appendix B. Fixed Dulcimer mapping

Source: [Dulcimer maps](../src/Instruments/DJthefirst/Dulcimer.h).

Only the following MIDI notes map to solenoids. Output numbers are **1-based** in the map; subtract one for the bank API. Outputs 1-16 belong to chain 1; 17-40 belong to chain 2, whose local index is `output - 17`. Output 6 is unmapped.

| MIDI note | Solenoid output | LED index |
| --- | --- | --- |
| 50 | 1 | 2 |
| 52 | 2 | 6 |
| 54 | 3 | 9 |
| 55 | 4 | 13 |
| 56 | 17 | 112 |
| 57 | 18 | 109 |
| 58 | 5 | 16 |
| 59 | 7 | 20 |
| 60 | 8 | 23 |
| 61 | 19 | 102 |
| 62 | 25 | 99 |
| 63 | 9 | 27 |
| 64 | 10 | 30 |
| 65 | 11 | 34 |
| 66 | 32 | 76 |
| 67 | 12 | 37 |
| 68 | 20 | 101 |
| 69 | 13 | 40 |
| 70 | 14 | 44 |
| 71 | 31 | 81 |
| 72 | 30 | 78 |
| 73 | 15 | 48 |
| 74 | 29 | 75 |
| 75 | 16 | 51 |
| 76 | 40 | 72 |
| 77 | 39 | 68 |
| 78 | 26 | 83 |
| 79 | 27 | 79 |
| 80 | 38 | 64 |
| 81 | 28 | 76 |
| 82 | 37 | 61 |
| 83 | 33 | 71 |
| 84 | 34 | 0 |
| 86 | 35 | 0 |
| 88 | 36 | 0 |

Other pitches have no solenoid mapping and are ignored by Dulcimer play/stop. LED index zero is also a real LED index, so using zero for absent LED mapping conflates "unmapped" with "first LED"; the replacement should represent absence explicitly while retaining these legacy values in its import profile.

The pulse timer is restarted on retrigger, active counts are shared across the 40-output bank, and Note Off can terminate a pulse before its configured 9 ms. Both banks are latched after play/stop; timeout service also updates LEDs. Physical driver polarity, current limits, flyback protection, cooldown and independent pulse cutoff are not specified by these maps and require the hardware profile described in sections 10 and 15.
