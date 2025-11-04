/*
 * NullLocalStorage.cpp
 * No-op implementation of local storage for platforms that don't support persistent storage
 */

#include "NullLocalStorage.h"
#include "Device.h"

std::string NullLocalStorage::getDeviceName() {
    // Return the compile-time default device name
    return Device::Name;
}

uint16_t NullLocalStorage::getDeviceID() {
    // Return the compile-time default device ID
    return Device::GetDeviceID();
}
