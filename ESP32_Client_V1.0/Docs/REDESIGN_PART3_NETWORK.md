# Part 3: Network Architecture Design

## Overview

The network layer handles all I/O for MIDI messages. This is an **I/O-bound operation** where virtual function overhead is negligible compared to actual network latency. The V1.0 virtual interface is the correct approach and should be preserved with minor enhancements.

## Performance Reality Check

### Network Operation Timings (ESP32 @ 240MHz)

| Operation | Time | Notes |
|-----------|------|-------|
| Virtual function call | ~5 ns | 1-2 CPU cycles |
| Serial.read() | ~100 μs | UART hardware latency |
| WiFi UDP packet | ~500 μs | Network stack overhead |
| BLE notification | ~1-10 ms | Bluetooth stack overhead |

**Conclusion**: Virtual function overhead is **0.001% to 0.0001%** of total I/O time. Optimization here provides zero benefit.

## Design Decision: Keep Virtual Functions

### Rationale

1. **I/O is the bottleneck**, not function calls
2. **Runtime flexibility** - switch networks without recompiling
3. **Code clarity** - polymorphic interface is intuitive
4. **Debugging ease** - swap implementations during development
5. **Multiple networks** - composition pattern works naturally

### What the Pure Template Approach Got Wrong

```cpp
// Pure template V2.0 (REJECTED) - Premature optimization
template<typename... NetworkTypes>
struct Network {
    static void begin() { (NetworkTypes::begin(), ...); }
    static void sendMessage(const MidiMessage& msg) { 
        (NetworkTypes::sendMessage(msg), ...); 
    }
};

// Problems:
// ❌ Template instantiation for every combination (code bloat)
// ❌ No runtime network switching
// ❌ Cryptic template errors
// ❌ Saves ~5 nanoseconds on 500 microsecond operation
// ❌ Harder to debug and test
```

## Improved V2.0 Network Design

### Base Interface (Keep V1.0 Design)

```cpp
// Network.h - Polymorphic base interface
#pragma once

#include "MidiMessage.h"
#include <optional>
#include <cstdint>

class Network {
protected:
    static constexpr uint8_t MIN_MSG_BYTES = 3;
    
public:
    virtual ~Network() = default;
    
    // Core interface
    virtual void begin() = 0;
    virtual void sendMessage(const MidiMessage& message) = 0;
    virtual std::optional<MidiMessage> readMessage() = 0;
    
    // Status and diagnostics
    virtual bool isConnected() const { return true; }
    virtual const char* getName() const = 0;
    virtual void printStatus() const {}
    
    // Optional: Advanced features
    virtual void sendString(const String& str) {}
    virtual void flush() {}
};
```

### Serial Network Implementation

```cpp
// NetworkSerial.h
#pragma once

#include "Network.h"
#include <Arduino.h>

class SerialNetwork : public Network {
private:
    static constexpr uint32_t BAUD_RATE = 115200;
    static constexpr uint8_t BUFFER_SIZE = 128;
    
    uint8_t m_buffer[BUFFER_SIZE];
    uint8_t m_bufferPos = 0;
    bool m_inSysEx = false;
    
public:
    void begin() override {
        Serial.begin(BAUD_RATE);
        while (!Serial && millis() < 3000) {
            delay(10);  // Wait for Serial with timeout
        }
    }
    
    void sendMessage(const MidiMessage& message) override {
        // Status byte
        Serial.write(message.status);
        
        // Data bytes (if any)
        if (message.data1 != MidiMessage::NO_DATA) {
            Serial.write(message.data1);
            
            if (message.data2 != MidiMessage::NO_DATA) {
                Serial.write(message.data2);
            }
        }
        
        // SysEx data
        if (message.isSysEx() && message.sysExLength > 0) {
            Serial.write(message.sysExData, message.sysExLength);
            Serial.write(0xF7);  // End SysEx
        }
    }
    
    std::optional<MidiMessage> readMessage() override {
        while (Serial.available()) {
            uint8_t byte = Serial.read();
            
            // Status byte (0x80-0xFF)
            if (byte & 0x80) {
                if (byte == 0xF0) {
                    // Start SysEx
                    m_inSysEx = true;
                    m_buffer[0] = byte;
                    m_bufferPos = 1;
                } else if (byte == 0xF7 && m_inSysEx) {
                    // End SysEx
                    m_inSysEx = false;
                    return parseSysEx();
                } else if (!m_inSysEx) {
                    // Regular MIDI message
                    m_buffer[0] = byte;
                    m_bufferPos = 1;
                    return parseMessage();
                }
            } else if (m_inSysEx) {
                // SysEx data byte
                if (m_bufferPos < BUFFER_SIZE) {
                    m_buffer[m_bufferPos++] = byte;
                }
            } else if (m_bufferPos > 0 && m_bufferPos < MIN_MSG_BYTES) {
                // Data byte for regular message
                m_buffer[m_bufferPos++] = byte;
                if (m_bufferPos >= getMessageLength(m_buffer[0])) {
                    return parseMessage();
                }
            }
        }
        
        return std::nullopt;
    }
    
    bool isConnected() const override {
        return Serial;
    }
    
    const char* getName() const override {
        return "Serial";
    }
    
    void printStatus() const override {
        Serial.printf("Serial Network: %d baud, %s\n", 
                      BAUD_RATE, 
                      isConnected() ? "Connected" : "Disconnected");
    }
    
private:
    std::optional<MidiMessage> parseMessage() {
        if (m_bufferPos < MIN_MSG_BYTES) return std::nullopt;
        
        MidiMessage msg;
        msg.status = m_buffer[0];
        msg.data1 = m_buffer[1];
        msg.data2 = (m_bufferPos > 2) ? m_buffer[2] : MidiMessage::NO_DATA;
        
        m_bufferPos = 0;
        return msg;
    }
    
    std::optional<MidiMessage> parseSysEx() {
        if (m_bufferPos < 2) return std::nullopt;
        
        MidiMessage msg;
        msg.status = 0xF0;
        msg.sysExLength = m_bufferPos - 1;  // Exclude 0xF0
        memcpy(msg.sysExData, &m_buffer[1], msg.sysExLength);
        
        m_bufferPos = 0;
        return msg;
    }
    
    uint8_t getMessageLength(uint8_t status) const {
        uint8_t type = status & 0xF0;
        switch (type) {
            case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0:
                return 3;  // Note On/Off, CC, Pitch Bend
            case 0xC0: case 0xD0:
                return 2;  // Program Change, Channel Pressure
            default:
                return 1;
        }
    }
};
```

### UDP Network Implementation (ESP32)

```cpp
// NetworkUDP.h
#pragma once

#include "Network.h"

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <WiFiUdp.h>

class UdpNetwork : public Network {
private:
    static constexpr uint16_t PORT = 65534;
    static constexpr uint16_t BUFFER_SIZE = 256;
    
    WiFiUDP m_udp;
    IPAddress m_serverIP;
    uint8_t m_buffer[BUFFER_SIZE];
    bool m_initialized = false;
    
public:
    UdpNetwork(IPAddress serverIP = IPAddress(192, 168, 1, 100))
        : m_serverIP(serverIP) {}
    
    void begin() override {
        // WiFi should already be initialized
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WARNING: UDP Network requires WiFi connection");
            return;
        }
        
        if (m_udp.begin(PORT)) {
            m_initialized = true;
            Serial.printf("UDP Network listening on port %d\n", PORT);
        } else {
            Serial.println("ERROR: Failed to start UDP");
        }
    }
    
    void sendMessage(const MidiMessage& message) override {
        if (!m_initialized) return;
        
        m_udp.beginPacket(m_serverIP, PORT);
        m_udp.write(message.status);
        
        if (message.data1 != MidiMessage::NO_DATA) {
            m_udp.write(message.data1);
            
            if (message.data2 != MidiMessage::NO_DATA) {
                m_udp.write(message.data2);
            }
        }
        
        if (message.isSysEx() && message.sysExLength > 0) {
            m_udp.write(message.sysExData, message.sysExLength);
            m_udp.write(0xF7);
        }
        
        m_udp.endPacket();
    }
    
    std::optional<MidiMessage> readMessage() override {
        if (!m_initialized) return std::nullopt;
        
        int packetSize = m_udp.parsePacket();
        if (packetSize == 0) return std::nullopt;
        
        int len = m_udp.read(m_buffer, BUFFER_SIZE);
        if (len < MIN_MSG_BYTES) return std::nullopt;
        
        MidiMessage msg;
        msg.status = m_buffer[0];
        
        if (msg.status == 0xF0) {
            // SysEx
            msg.sysExLength = len - 2;  // Exclude F0 and F7
            memcpy(msg.sysExData, &m_buffer[1], msg.sysExLength);
        } else {
            // Regular message
            msg.data1 = (len > 1) ? m_buffer[1] : MidiMessage::NO_DATA;
            msg.data2 = (len > 2) ? m_buffer[2] : MidiMessage::NO_DATA;
        }
        
        return msg;
    }
    
    bool isConnected() const override {
        return m_initialized && WiFi.status() == WL_CONNECTED;
    }
    
    const char* getName() const override {
        return "UDP";
    }
    
    void printStatus() const override {
        Serial.printf("UDP Network: %s, Server: %s:%d\n",
                      isConnected() ? "Connected" : "Disconnected",
                      m_serverIP.toString().c_str(),
                      PORT);
    }
    
    void setServerIP(IPAddress ip) {
        m_serverIP = ip;
    }
};

#endif  // ARDUINO_ARCH_ESP32
```

### BLE Network Implementation (ESP32)

```cpp
// NetworkBLE.h
#pragma once

#include "Network.h"

#ifdef ARDUINO_ARCH_ESP32
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BleNetwork : public Network, public BLEServerCallbacks, public BLECharacteristicCallbacks {
private:
    static constexpr const char* SERVICE_UUID = "03b80e5a-ede8-4b33-a751-6ce34ec4c700";
    static constexpr const char* CHAR_UUID_RX = "7772e5db-3868-4112-a1a9-f2669d106bf3";
    static constexpr const char* CHAR_UUID_TX = "7772e5db-3868-4112-a1a9-f2669d106bf4";
    
    BLEServer* m_server = nullptr;
    BLECharacteristic* m_rxChar = nullptr;
    BLECharacteristic* m_txChar = nullptr;
    bool m_connected = false;
    
    String m_deviceName;
    std::vector<MidiMessage> m_messageQueue;
    
public:
    explicit BleNetwork(const char* deviceName = "MMM Device")
        : m_deviceName(deviceName) {}
    
    void begin() override {
        BLEDevice::init(m_deviceName.c_str());
        
        m_server = BLEDevice::createServer();
        m_server->setCallbacks(this);
        
        BLEService* service = m_server->createService(SERVICE_UUID);
        
        // RX characteristic (receive from client)
        m_rxChar = service->createCharacteristic(
            CHAR_UUID_RX,
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR
        );
        m_rxChar->setCallbacks(this);
        
        // TX characteristic (send to client)
        m_txChar = service->createCharacteristic(
            CHAR_UUID_TX,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        m_txChar->addDescriptor(new BLE2902());
        
        service->start();
        
        BLEAdvertising* advertising = BLEDevice::getAdvertising();
        advertising->addServiceUUID(SERVICE_UUID);
        advertising->setScanResponse(true);
        advertising->start();
        
        Serial.printf("BLE Network started: %s\n", m_deviceName.c_str());
    }
    
    void sendMessage(const MidiMessage& message) override {
        if (!m_connected || !m_txChar) return;
        
        uint8_t data[32];
        uint8_t len = 0;
        
        data[len++] = message.status;
        if (message.data1 != MidiMessage::NO_DATA) {
            data[len++] = message.data1;
            if (message.data2 != MidiMessage::NO_DATA) {
                data[len++] = message.data2;
            }
        }
        
        if (message.isSysEx() && message.sysExLength > 0) {
            memcpy(&data[len], message.sysExData, 
                   min(message.sysExLength, 32 - len));
            len += message.sysExLength;
            data[len++] = 0xF7;
        }
        
        m_txChar->setValue(data, len);
        m_txChar->notify();
    }
    
    std::optional<MidiMessage> readMessage() override {
        if (m_messageQueue.empty()) return std::nullopt;
        
        MidiMessage msg = m_messageQueue.front();
        m_messageQueue.erase(m_messageQueue.begin());
        return msg;
    }
    
    bool isConnected() const override {
        return m_connected;
    }
    
    const char* getName() const override {
        return "BLE";
    }
    
    void printStatus() const override {
        Serial.printf("BLE Network: %s (%s)\n",
                      m_connected ? "Connected" : "Advertising",
                      m_deviceName.c_str());
    }
    
    // BLE Callbacks
    void onConnect(BLEServer* server) override {
        m_connected = true;
        Serial.println("BLE Client connected");
    }
    
    void onDisconnect(BLEServer* server) override {
        m_connected = false;
        Serial.println("BLE Client disconnected");
        BLEDevice::startAdvertising();  // Resume advertising
    }
    
    void onWrite(BLECharacteristic* characteristic) override {
        std::string value = characteristic->getValue();
        if (value.length() < MIN_MSG_BYTES) return;
        
        MidiMessage msg;
        msg.status = value[0];
        msg.data1 = (value.length() > 1) ? value[1] : MidiMessage::NO_DATA;
        msg.data2 = (value.length() > 2) ? value[2] : MidiMessage::NO_DATA;
        
        if (msg.status == 0xF0 && value.length() > 3) {
            msg.sysExLength = value.length() - 2;
            memcpy(msg.sysExData, &value[1], msg.sysExLength);
        }
        
        m_messageQueue.push_back(msg);
    }
};

#endif  // ARDUINO_ARCH_ESP32
```

## Multiple Network Support

### Composition Pattern (Recommended)

```cpp
// MultiNetwork.h - Support multiple networks simultaneously
#pragma once

#include "Network.h"
#include <vector>
#include <memory>

class MultiNetwork : public Network {
private:
    std::vector<std::unique_ptr<Network>> m_networks;
    
public:
    MultiNetwork() = default;
    
    // Add networks during construction
    template<typename NetworkType, typename... Args>
    void addNetwork(Args&&... args) {
        m_networks.push_back(
            std::make_unique<NetworkType>(std::forward<Args>(args)...)
        );
    }
    
    void begin() override {
        for (auto& net : m_networks) {
            net->begin();
        }
    }
    
    void sendMessage(const MidiMessage& message) override {
        // Broadcast to all networks
        for (auto& net : m_networks) {
            if (net->isConnected()) {
                net->sendMessage(message);
            }
        }
    }
    
    std::optional<MidiMessage> readMessage() override {
        // Check networks in priority order (first added = highest priority)
        for (auto& net : m_networks) {
            if (net->isConnected()) {
                auto msg = net->readMessage();
                if (msg.has_value()) {
                    return msg;
                }
            }
        }
        return std::nullopt;
    }
    
    bool isConnected() const override {
        // Connected if any network is connected
        for (const auto& net : m_networks) {
            if (net->isConnected()) return true;
        }
        return false;
    }
    
    const char* getName() const override {
        return "MultiNetwork";
    }
    
    void printStatus() const override {
        Serial.println("=== Network Status ===");
        for (const auto& net : m_networks) {
            net->printStatus();
        }
    }
    
    size_t getNetworkCount() const {
        return m_networks.size();
    }
};
```

### Usage in Configuration

```cpp
// config.h - Multiple network example
#pragma once

#define DEVICE_NAME "Multi-Network Device"
#define DEVICE_ID 0x0042
#define PINS 2,4,12,13,14,15,16,17

#include "instruments/FloppyDrive.h"
using InstrumentType = FloppyDrive;

// Enable multiple networks
#define ENABLE_MULTI_NETWORK
```

```cpp
// main.cpp - MultiNetwork setup
#include "config.h"

#ifdef ENABLE_MULTI_NETWORK
    #include "networks/MultiNetwork.h"
    #include "networks/SerialNetwork.h"
    #include "networks/UdpNetwork.h"
    
    MultiNetwork network;
    
    void setupNetwork() {
        network.addNetwork<SerialNetwork>();
        network.addNetwork<UdpNetwork>(IPAddress(192, 168, 1, 100));
        network.begin();
    }
#else
    #include "networks/SerialNetwork.h"
    SerialNetwork network;
    
    void setupNetwork() {
        network.begin();
    }
#endif
```

## Network Selection Strategy

### Decision Tree

```
Is network I/O performance-critical? → NO (I/O-bound, not CPU-bound)
   ↓
Do you need runtime network switching? → YES (debug vs production)
   ↓
Do you need multiple simultaneous networks? → MAYBE (Serial + WiFi)
   ↓
CONCLUSION: Use polymorphic virtual interface with composition
```

### Memory Cost Analysis

| Approach | Flash Cost | RAM Cost | Runtime Overhead |
|----------|-----------|----------|------------------|
| Virtual Functions (V1.0) | +200 bytes | +40 bytes | 5 ns per call |
| Templates (Pure V2.0) | +500-1500 bytes | +0 bytes | 0 ns per call |
| **Hybrid (Recommended)** | +200 bytes | +40 bytes | 5 ns per call |

**Verdict**: Virtual functions win on both code size and flexibility. Template version actually INCREASES flash usage due to instantiation combinatorics.

## Benefits of This Design

### 1. Performance
- ✅ Virtual overhead negligible (0.001% of I/O time)
- ✅ No template code bloat
- ✅ Efficient message parsing

### 2. Flexibility
- ✅ Runtime network switching (debug ↔ production)
- ✅ Multiple simultaneous networks
- ✅ Easy to add new network types
- ✅ Platform-specific implementations

### 3. Usability
- ✅ Clear polymorphic interface
- ✅ Easy to test and debug
- ✅ Standard C++ patterns
- ✅ Intuitive for new developers

### 4. Maintainability
- ✅ Separation of concerns
- ✅ Each network self-contained
- ✅ No template complexity
- ✅ Clear error messages

## Testing Strategy

### Unit Tests

```cpp
// test_network.cpp
#include <gtest/gtest.h>
#include "networks/SerialNetwork.h"

TEST(SerialNetwork, ParsesNoteOn) {
    // Mock Serial interface
    MockSerial serial;
    serial.addBytes({0x90, 0x3C, 0x7F});  // Note On C4, velocity 127
    
    SerialNetwork net;
    auto msg = net.readMessage();
    
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg->status, 0x90);
    EXPECT_EQ(msg->data1, 0x3C);
    EXPECT_EQ(msg->data2, 0x7F);
}

TEST(MultiNetwork, BroadcastsToAll) {
    MultiNetwork net;
    net.addNetwork<MockNetwork>("Net1");
    net.addNetwork<MockNetwork>("Net2");
    
    MidiMessage msg{0x90, 0x3C, 0x7F};
    net.sendMessage(msg);
    
    // Verify both networks received message
    // ...
}
```

## Migration from V1.0

### Changes Required

**Minimal** - V1.0 network interface is already correct!

Only additions:
1. Add `getName()` method to existing networks
2. Optional: Add `printStatus()` for diagnostics
3. Optional: Implement MultiNetwork for multiple simultaneous connections

```cpp
// V1.0 Network (works as-is)
class SerialNetwork : public Network {
    void begin() override { /* existing code */ }
    void sendMessage(MidiMessage msg) override { /* existing code */ }
    std::optional<MidiMessage> readMessage() override { /* existing code */ }
};

// V2.0 Addition (optional enhancement)
class SerialNetwork : public Network {
    // ... existing methods ...
    
    const char* getName() const override { return "Serial"; }  // NEW
    void printStatus() const override {                        // NEW
        Serial.printf("Serial: %s\n", isConnected() ? "OK" : "Disconnected");
    }
};
```

## Next Steps

With network architecture defined, we can proceed to:
- **Part 4**: Instrument and Controller hybrid architecture
- **Part 5**: Distributor system with runtime management
- **Part 6**: Implementation strategy and migration path
