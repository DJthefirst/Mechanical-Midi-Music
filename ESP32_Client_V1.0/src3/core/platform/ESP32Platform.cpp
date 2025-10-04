/*
 * ESP32Platform.cpp - ESP32 Platform Implementation
 *
 * Part of the Mechanical MIDI Music V2.0 redesign
 * Based on REDESIGN_PART6_IMPLEMENTATION.md specifications
 */

#ifdef ARDUINO_ARCH_ESP32

#include "PlatformInterface.h"
#include <Arduino.h>
#include <esp_system.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// ESP32 Platform Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

void Platform::init() {
    // ESP32-specific initialization
    // Most initialization is handled by Arduino framework
    Serial.println("ESP32 Platform initialized");
}

uint32_t Platform::micros() {
    return ::micros();
}

uint32_t Platform::millis() {
    return ::millis();
}

void Platform::delayMicroseconds(uint32_t us) {
    ::delayMicroseconds(us);
}

void Platform::delayMilliseconds(uint32_t ms) {
    ::delay(ms);
}

PlatformCapabilities Platform::getCapabilities() {
    return {
        .hasHardwarePWM = true,
        .hasWiFi = true,
        .hasBLE = true,
        .hasNVS = true,
        .hasHardwareTimer = true,
        .maxFlashSize = 4 * 1024 * 1024,  // 4MB typical
        .maxRAM = 520 * 1024,             // 520KB usable RAM
        .maxFrequency = 240000000,        // 240MHz max
        .maxPinNumber = 39                // GPIO 0-39
    };
}

const char* Platform::getName() {
    return "ESP32";
}

const char* Platform::getVersion() {
    return "V2.0";
}

bool Platform::isValidPin(uint8_t pin) {
    // ESP32 has GPIO 0-39, but some are restricted
    if (pin > 39) return false;
    
    // Pins used for flash (avoid these)
    if (pin >= 6 && pin <= 11) return false;
    
    // Other restricted pins
    if (pin == 0 || pin == 1) return false;  // Serial0
    
    return true;
}

bool Platform::pinSupportsPWM(uint8_t pin) {
    if (!isValidPin(pin)) return false;
    
    // Input-only pins don't support PWM
    if (pin >= 34 && pin <= 39) return false;
    
    return true;
}

bool Platform::pinIsInputOnly(uint8_t pin) {
    return (pin >= 34 && pin <= 39);
}

uint32_t Platform::getFreeHeap() {
    return ESP.getFreeHeap();
}

uint32_t Platform::getMinFreeHeap() {
    return ESP.getMinFreeHeap();
}

void Platform::feedWatchdog() {
    // Use Arduino framework watchdog feed function
    yield();  // Equivalent to feeding watchdog in Arduino ESP32
}

void Platform::restart() {
    ESP.restart();
}

void Platform::printInfo() {
    auto caps = getCapabilities();
    
    Serial.println("=== Platform Information ===");
    Serial.printf("Platform: %s %s\n", getName(), getVersion());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %d MB\n", caps.maxFlashSize / (1024 * 1024));
    Serial.printf("Free Heap: %d bytes\n", getFreeHeap());
    Serial.printf("Min Free Heap: %d bytes\n", getMinFreeHeap());
    
    Serial.println("Capabilities:");
    Serial.printf("  Hardware PWM: %s\n", caps.hasHardwarePWM ? "Yes" : "No");
    Serial.printf("  WiFi: %s\n", caps.hasWiFi ? "Yes" : "No");
    Serial.printf("  BLE: %s\n", caps.hasBLE ? "Yes" : "No");
    Serial.printf("  NVS: %s\n", caps.hasNVS ? "Yes" : "No");
    Serial.printf("  Hardware Timer: %s\n", caps.hasHardwareTimer ? "Yes" : "No");
    Serial.printf("  Max GPIO Pin: %d\n", caps.maxPinNumber);
    
    // Chip information
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("SDK Version: %s\n", ESP.getSdkVersion());
}

#endif  // ARDUINO_ARCH_ESP32