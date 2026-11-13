#include <Arduino.h>
#include "ESP32_S3_NetworkUSB.h"

#if defined(CFG_MMM_NETWORK_USB) && defined(CFG_ESP32_S3)

#include "Config.h"
#include "USB.h"
#include "esp32-hal-tinyusb.h"

#if !CONFIG_TINYUSB_MIDI_ENABLED || ARDUINO_USB_MODE
    #error "ESP32-S3 USB MIDI requires TinyUSB MIDI support and ARDUINO_USB_MODE=0"
#endif
#if ARDUINO_USB_ON_BOOT
    #error "Disable USB CDC/MSC/DFU on boot so NetworkUSB can register MIDI before USB starts"
#endif

namespace {
uint16_t loadMidiDescriptor(uint8_t* destination, uint8_t* interfaceNumber) {
    const uint8_t stringIndex = tinyusb_add_string_descriptor(CFG_DEVICE_NAME);
    const uint8_t endpoint = tinyusb_get_free_duplex_endpoint();
    if (stringIndex == 0 || endpoint == 0) {
        log_e("USB MIDI descriptor allocation failed");
        return 0;
    }
    const uint8_t descriptor[] = {
        TUD_MIDI_DESCRIPTOR(*interfaceNumber, stringIndex, endpoint,
                            static_cast<uint8_t>(0x80 | endpoint), 64)
    };
    std::copy_n(descriptor, TUD_MIDI_DESC_LEN, destination);
    *interfaceNumber += 2;
    return TUD_MIDI_DESC_LEN;
}
}

void ESP32_S3_NetworkUSB::begin() {
    if (m_started) return;
    #ifndef CFG_MMM_NETWORK_SERIAL
        Serial.begin(115200);
    #endif
    const esp_err_t error = tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, loadMidiDescriptor);
    if (error != ESP_OK) {
        log_e("USB MIDI registration failed: %s", esp_err_to_name(error));
        return;
    }
    if (!USB.productName(CFG_DEVICE_NAME) || !USB.usbClass(0) ||
        !USB.usbSubClass(0) || !USB.usbProtocol(0)) {
        log_e("USB MIDI device descriptor setup failed");
        return;
    }
    #ifdef CFG_USB_MIDI_PID
        if (!USB.PID(CFG_USB_MIDI_PID)) {
            log_e("USB MIDI product ID setup failed");
            return;
        }
    #endif
    m_started = USB.begin();
    if (!m_started) log_e("USB MIDI startup failed");
}

std::optional<MidiMessage> ESP32_S3_NetworkUSB::readMessage() {
    if (!m_started || !tud_midi_mounted()) {
        m_codec.reset();
        return std::nullopt;
    }
    ESP32_S3_USBMidiCodec::Packet packet;
    while (tud_midi_packet_read(packet.data())) {
        auto result = m_codec.decodePacket(packet);
        if (result.error == ESP32_S3_USBMidiCodec::Error::InvalidPacket) {
            log_e("Invalid USB MIDI packet");
        } else if (result.error == ESP32_S3_USBMidiCodec::Error::SysExOverflow) {
            log_e("USB MIDI SysEx exceeds %u bytes; discarded", MAX_PACKET_LENGTH);
        }
        if (result.message) return result.message;
    }
    return std::nullopt;
}

void ESP32_S3_NetworkUSB::sendMessage(const MidiMessage& message) {
    if (!ESP32_S3_USBMidiCodec::isValid(message)) {
        log_e("Invalid outgoing USB MIDI message");
        return;
    }
    if (!m_started || !tud_midi_mounted()) {
        log_e("USB MIDI host is not connected; message not sent");
        return;
    }
    // One deadline bounds the entire send, including a fragmented SysEx.
    const uint32_t start = millis();
    uint8_t offset = 0;
    while (offset < message.length) {
        const auto packet = ESP32_S3_USBMidiCodec::encodePacket(message, offset);
        while (!tud_midi_packet_write(packet.data())) {
            if (!tud_midi_mounted() || millis() - start >= 20) {
                log_e("USB MIDI transmit failed or timed out");
                return;
            }
            delay(1);
        }
    }
}

void ESP32_S3_NetworkUSB::sendString(const String& message) {
    // Text uses the UART/debug port, never the MIDI endpoint.
    // NetworkSerial already prints to the same UART when enabled.
#ifndef CFG_MMM_NETWORK_SERIAL
    Serial.println(message);
#else
    (void)message;
#endif
}

#endif
