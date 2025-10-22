# MIDI 7-bit Encoding Utility Guide

## Overview

MIDI SysEx messages require all data bytes to have MSB (bit 7) = 0, limiting each byte to values 0x00-0x7F. This means multi-byte values must be encoded/decoded specially.

## Utility Functions (in `Utility.h`)

### `encodeTo7Bit<NumBits>(bitset)`

Encodes an N-bit bitset into MIDI 7-bit byte array.

```cpp
template <size_t NumBits>
std::array<uint8_t, (NumBits + 6) / 7> encodeTo7Bit(const std::bitset<NumBits>& bits)
```

**Example:**
```cpp
std::bitset<16> channels = 0x1234;
auto midiBytes = Utility::encodeTo7Bit(channels);
// midiBytes is std::array<uint8_t, 3> with all values < 0x80
```

### `decodeFrom7Bit<NumBits>(midiBytes)`

Decodes MIDI 7-bit bytes back to an N-bit bitset.

```cpp
template <size_t NumBits>
std::bitset<NumBits> decodeFrom7Bit(const uint8_t* midiBytes)
```

**Example:**
```cpp
uint8_t midiData[] = {0x34, 0x24, 0x09};
auto channels = Utility::decodeFrom7Bit<16>(midiData);
// channels is std::bitset<16> = 0x1234
```

## Size Reference

| Bits | MIDI Bytes | Formula |
|------|------------|---------|
| 7    | 1          | (7+6)/7 = 1 |
| 8    | 2          | (8+6)/7 = 2 |
| 14   | 2          | (14+6)/7 = 2 |
| 15   | 3          | (15+6)/7 = 3 |
| 16   | 3          | (16+6)/7 = 3 |
| 32   | 5          | (32+6)/7 = 5 |
| 64   | 10         | (64+6)/7 = 10 |

## Usage Examples

### Encoding Device ID (14 bits → 2 MIDI bytes)
```cpp
std::bitset<14> deviceID = 0x3FFF;
auto midiBytes = Utility::encodeTo7Bit(deviceID);
// Send: midiBytes[0], midiBytes[1]
```

### Encoding Channels (16 bits → 3 MIDI bytes)
```cpp
std::bitset<16> channels = 0x0003; // Channels 0 and 1 enabled
auto midiBytes = Utility::encodeTo7Bit(channels);
distributorObj[2] = midiBytes[0];
distributorObj[3] = midiBytes[1];
distributorObj[4] = midiBytes[2];
```

### Encoding Instruments (32 bits → 5 MIDI bytes)
```cpp
std::bitset<32> instruments = 0x00000301; // Instruments 0, 8, 9 enabled
auto midiBytes = Utility::encodeTo7Bit(instruments);
distributorObj[5] = midiBytes[0];
distributorObj[6] = midiBytes[1];
distributorObj[7] = midiBytes[2];
distributorObj[8] = midiBytes[3];
distributorObj[9] = midiBytes[4];
```

### Decoding on Receive
```cpp
// Received MIDI SysEx payload at &data[2]
auto channels = Utility::decodeFrom7Bit<16>(&data[2]);
auto instruments = Utility::decodeFrom7Bit<32>(&data[5]);

// Convert to uint if needed
uint16_t channelsInt = channels.to_ulong();
uint32_t instrumentsInt = instruments.to_ulong();
```

## How It Works

The encoding packs bits sequentially into 7-bit chunks:

```
Input bits:  [b15 b14 b13 b12 b11 b10 b9  b8  b7  b6  b5  b4  b3  b2  b1  b0]
             └────────────┬────────────┘  └──────────┬──────────┘  └─────┬────┘
                          │                           │                   │
MIDI Byte 0: [0 b6  b5  b4  b3  b2  b1  b0 ]  ←──────┘                   │
MIDI Byte 1: [0 b13 b12 b11 b10 b9  b8  b7 ]  ←──────────────────────────┘
MIDI Byte 2: [0 0   0   0   0   b15 b14 0  ]  ←──────────────────────────────┘
```

All output bytes have MSB=0, making them valid MIDI data bytes.

## Testing

Verify round-trip encoding/decoding:
```cpp
std::bitset<16> original = 0x1234;
auto encoded = Utility::encodeTo7Bit(original);
auto decoded = Utility::decodeFrom7Bit<16>(encoded.data());
assert(original == decoded); // Should pass
```

## Common Pitfall to Avoid

❌ **DON'T** use `bitsetToBytes()` for MIDI transmission:
```cpp
auto bytes = Utility::bitsetToBytes(channels); // Returns 8-bit bytes!
// bytes may contain values > 0x7F, invalid for MIDI SysEx
```

✅ **DO** use `encodeTo7Bit()` for MIDI transmission:
```cpp
auto midiBytes = Utility::encodeTo7Bit(channels); // Returns 7-bit MIDI bytes
// All bytes guaranteed < 0x80
```

## Advantages

1. **Type Safety**: Template parameters catch size mismatches at compile time
2. **Correctness**: Single implementation reduces bugs
3. **Readability**: Intent is clear from function names
4. **Maintainability**: Changes to encoding logic only need to happen once
5. **Testability**: Can be unit tested independently
6. **Reusability**: Works for any bit size

## Future Enhancements

Consider adding:
- `encodeTo7BitArray()` - encode multiple values at once
- `decodeTo7BitArray()` - decode multiple values at once
- Unit tests with known test vectors
- Compile-time verification of MIDI byte constraints
