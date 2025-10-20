# Quick Reference: 8-bit vs 7-bit Conversion

## When to Use Each Format

| Context | Format | Why |
|---------|--------|-----|
| Distributor internal state | 8-bit | Native, efficient |
| LocalStorage (NVS/EEPROM) | 8-bit | Space efficient |
| DistributorManager operations | 8-bit | No conversion overhead |
| MIDI SysEx transmission | 7-bit | MIDI spec compliance |
| MIDI SysEx reception | 7-bit | MIDI spec compliance |

## Conversion Points

### Outgoing MIDI (8-bit → 7-bit)
```cpp
// In SysExMsgHandler when sending responses
auto data8bit = distributorManager->getDistributorSerial(id);
std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> data7bit;
Distributor::convertTo7Bit(data8bit.data(), data7bit.data());
// Send data7bit via MIDI
```

### Incoming MIDI (7-bit → 8-bit)
```cpp
// In SysExMsgHandler when receiving commands
std::array<uint8_t, DISTRIBUTOR_NUM_CFG_BYTES> data8bit;
Distributor::convertFrom7Bit(message.sysExCmdPayload(), data8bit.data());
distributorManager->setDistributor(data8bit.data());
```

## Byte Layout Reference

### 8-bit Internal Format
```
Index | Field              | Size  | Format
------|-------------------|-------|--------
0-1   | Distributor ID    | 2B    | uint16 LE
2-3   | Channels          | 2B    | uint16 LE
4-7   | Instruments       | 4B    | uint32 LE
8     | Method            | 1B    | uint8
9     | Booleans          | 1B    | uint8
10    | Min Note          | 1B    | uint8
11    | Max Note          | 1B    | uint8
12    | Num Poly Notes    | 1B    | uint8
13-15 | Reserved          | 3B    | 0x00

Total: 16 bytes
```

### 7-bit MIDI Format
```
Index | Field              | Size  | Format
------|-------------------|-------|--------
0-1   | Distributor ID    | 2B    | 7-bit
2-4   | Channels          | 3B    | encodeTo7Bit<16>
5-9   | Instruments       | 5B    | encodeTo7Bit<32>
10    | Method            | 1B    | 7-bit
11    | Booleans          | 1B    | 7-bit
12    | Min Note          | 1B    | 7-bit
13    | Max Note          | 1B    | 7-bit
14    | Num Poly Notes    | 1B    | 7-bit
15    | Reserved          | 1B    | 0x00

Total: 16 bytes (but with 7-bit data)
```

## Common Patterns

### Pattern 1: Creating Distributor from MIDI
```cpp
void handleMidiDistributor(const uint8_t* midi7bit) {
    uint8_t data8bit[DISTRIBUTOR_NUM_CFG_BYTES];
    Distributor::convertFrom7Bit(midi7bit, data8bit);
    
    Distributor dist;
    dist.setDistributor(data8bit);
    // dist is now ready to use
}
```

### Pattern 2: Sending Distributor via MIDI
```cpp
void sendDistributorViaMidi(const Distributor& dist) {
    auto data8bit = dist.toSerial();
    uint8_t data7bit[DISTRIBUTOR_NUM_CFG_BYTES];
    Distributor::convertTo7Bit(data8bit.data(), data7bit);
    // Send data7bit via MIDI SysEx
}
```

### Pattern 3: Saving to Storage
```cpp
void saveToStorage(const Distributor& dist, uint8_t id) {
    auto data8bit = dist.toSerial();
    // data8bit is already in the right format!
    LocalStorage::get().SetDistributorConstruct(id, data8bit.data());
}
```

### Pattern 4: Loading from Storage
```cpp
void loadFromStorage(Distributor& dist, uint8_t id) {
    uint8_t data8bit[DISTRIBUTOR_NUM_CFG_BYTES];
    LocalStorage::get().GetDistributorConstruct(id, data8bit);
    // data8bit is already in the right format!
    dist.setDistributor(data8bit);
}
```

## Validation

### Check if Data is 7-bit MIDI
```cpp
bool is7BitMidi(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (data[i] & 0x80) return false; // MSB set = not 7-bit
    }
    return true;
}
```

### Check if Channels/Instruments Need Encoding
```cpp
// Channels > 127 or Instruments > 127 need multi-byte encoding
bool needsEncoding(uint32_t value) {
    return value > 0x7F;
}
```

## Debugging Tips

### Print Format
```cpp
void debugPrint8bit(const uint8_t* data) {
    Serial.println("8-bit format:");
    for (int i = 0; i < 16; i++) {
        Serial.printf("%02X ", data[i]); // Can show bytes > 0x7F
    }
}

void debugPrint7bit(const uint8_t* data) {
    Serial.println("7-bit MIDI format:");
    for (int i = 0; i < 16; i++) {
        Serial.printf("%02X ", data[i]); // All bytes should be < 0x80
        if (data[i] & 0x80) Serial.print("⚠️"); // Flag violations
    }
}
```

### Verify Round-Trip
```cpp
void testRoundTrip(const uint8_t* original8bit) {
    uint8_t temp7bit[DISTRIBUTOR_NUM_CFG_BYTES];
    uint8_t decoded8bit[DISTRIBUTOR_NUM_CFG_BYTES];
    
    Distributor::convertTo7Bit(original8bit, temp7bit);
    Distributor::convertFrom7Bit(temp7bit, decoded8bit);
    
    if (memcmp(original8bit, decoded8bit, DISTRIBUTOR_NUM_CFG_BYTES) == 0) {
        Serial.println("✓ Round-trip successful");
    } else {
        Serial.println("✗ Round-trip FAILED");
    }
}
```

## Remember

🔴 **NEVER** send 8-bit data over MIDI (will violate spec)  
🟢 **ALWAYS** convert to 7-bit before MIDI transmission

🔴 **NEVER** store 7-bit data in LocalStorage (wastes space)  
🟢 **ALWAYS** use 8-bit format for storage

🔴 **NEVER** mix formats in business logic  
🟢 **ALWAYS** convert at the boundary (SysEx handler)
