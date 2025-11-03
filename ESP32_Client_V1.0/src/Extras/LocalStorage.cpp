//TODO: Add Error handling

#include "Device.h"
#ifdef EXTRA_LOCAL_STORAGE

#include <iostream>
#include <string>
#include "LocalStorage.h"
#include "Distributors/Distributor.h"
#include "Distributors/DistributorManager.h"


esp_err_t err;
nvs_handle_t handle;


//Requires Serial.begin before use
LocalStorage::LocalStorage(){
    // Initialize NVS
    err = nvs_flash_init();
    //Serial.printf("[NVS] Init result: %s\n", esp_err_to_name(err));
    
    // Only erase if absolutely necessary (version mismatch or no free pages)
    // This will cause data loss but is required for NVS to function
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // WARNING: This will erase all stored configuration
        //Serial.printf("[NVS] Erasing flash due to: %s\n", esp_err_to_name(err));
        err = nvs_flash_erase();
        if (err == ESP_OK) {
            err = nvs_flash_init();
            //Serial.printf("[NVS] Re-init after erase: %s\n", esp_err_to_name(err));
        }
    }
    
    // If initialization still fails, we can't use NVS but shouldn't crash
    // Device will run with default configuration
    if (err != ESP_OK) {
        //Serial.printf("[NVS] Init FAILED: %s - using defaults\n", esp_err_to_name(err));
    } else {
        //Serial.printf("[NVS] Init SUCCESS\n");
    }
}

bool LocalStorage::OpenNvs(){

    // Open
    err = nvs_open("storage", NVS_READWRITE, &handle);
    //if (err != ESP_OK) Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    if (err != ESP_OK) return(false);
    return(true);
}

//Load blob data sructure from flash memory
esp_err_t LocalStorage::ReadNvsBlob(const char *key, uint8_t* result, uint8_t arrayLength){
    
    size_t arraySize = sizeof(uint8_t)*arrayLength;
    uint8_t tempResult[arrayLength];

    OpenNvs();

    err = nvs_get_blob(handle, key, tempResult, &arraySize);

    //if (err != ESP_OK) Serial.printf("Error (%s) in ReadNvsBlob!\n", esp_err_to_name(err));
    switch (err) {
        case ESP_OK:
            std::copy(tempResult, tempResult+arraySize, result);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            //TODO
            break;
    }
    nvs_close(handle);
    return(err);
}

uint8_t LocalStorage::ReadNvsU8(const char *key, uint8_t defaultValue){
    uint8_t value;

    OpenNvs();
    err = nvs_get_u8(handle, key, &value);
    //if (err != ESP_OK) Serial.printf("Error (%s) in ReadNvsU8!\n", esp_err_to_name(err));
    nvs_close(handle);

    switch (err) {
        case ESP_OK:
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            OpenNvs();
            WriteNvsU8(key, defaultValue);
            nvs_close(handle);
            value = defaultValue;
            break;
    }
    return(value);
}

esp_err_t LocalStorage::WriteNvsBlob(const char *key, const uint8_t* data, uint8_t arrayLength){
    size_t arraySize = sizeof(uint8_t)*arrayLength;
    
    OpenNvs();
    err = nvs_set_blob(handle, key, data, arraySize);
    if (err == ESP_OK) err = nvs_commit(handle);
    nvs_close(handle);
    return(err);
}

void LocalStorage::WriteNvsU8(const char *key, uint8_t value){
    OpenNvs();
    err = nvs_set_u8(handle, key, value);
    //if (err != ESP_OK) Serial.printf("Error (%s) in setU8!\n", esp_err_to_name(err));
    err = nvs_commit(handle);
    //if (err != ESP_OK) Serial.printf("Error (%s) in SetU8Commit!\n", esp_err_to_name(err));
    nvs_close(handle);
}

bool LocalStorage::EnsureNVSInitialized(){
    // Check if NVS is properly initialized by attempting a simple operation
    nvs_handle_t testHandle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &testHandle);
    
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        // NVS is not initialized, try to initialize it
        err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // This would require erasing, but that means losing data
            // Return false to use defaults instead
            return false;
        }
        if (err != ESP_OK) {
            return false;
        }
    } else if (err == ESP_OK) {
        // NVS is working, close the test handle
        nvs_close(testHandle);
    }
    // If err is ESP_ERR_NVS_NOT_FOUND, that's normal - the "storage" namespace doesn't exist yet
    
    return true;
}

void LocalStorage::ResetDeviceConfig(){
    // Clear device configuration by removing specific keys rather than erasing entire flash
    OpenNvs();
    
    // Erase device-specific keys
    nvs_erase_key(handle, "Device_name");
    nvs_erase_key(handle, "Device_id");
    nvs_erase_key(handle, "Device_bool");
    nvs_erase_key(handle, "Distributor_num");
    
    // Erase all distributor constructs (support up to 256 distributors)
    for (uint16_t i = 0; i < 256; i++) {
        std::string key = Uint16ToKey(i);
        nvs_erase_key(handle, key.c_str());
    }
    
    // Commit changes
    nvs_commit(handle);
    nvs_close(handle);
}

//TODO Use Smart Pointer?
std::string LocalStorage::GetDeviceName(){
    // Read a fixed-size, space-padded name blob from NVS and convert to std::string
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    // Initialize with null bytes to ensure trailing bytes are padded if stored blob is shorter
    memset(tmp, 0x00, DEVICE_NUM_NAME_BYTES);
    err = ReadNvsBlob("Device_name", tmp, DEVICE_NUM_NAME_BYTES);
    //Serial.printf("[NVS] GetDeviceName: %s\n", esp_err_to_name(err));

    // If not found or any error occurred, return default
    if (err != ESP_OK){
        //Serial.printf("[NVS] Using default name: '%s'\n", Device::Name.c_str());
        return Device::Name;
    }

    // Convert to std::string and trim trailing spaces and null bytes
    std::string name(reinterpret_cast<char*>(tmp), DEVICE_NUM_NAME_BYTES);
    size_t end = name.find_last_not_of(" \0");
    if (end != std::string::npos) {
        name.erase(end + 1);
    } else {
        // If name is all spaces/nulls, return default instead of empty string
        //Serial.printf("[NVS] Stored name empty, using default: '%s'\n", Device::Name.c_str());
        return Device::Name;
    }

    //Serial.printf("[NVS] Loaded name: '%s'\n", name.c_str());
    return name;
}

void LocalStorage::SetDeviceName(std::string name){ 
    // Write a fixed-size 20-byte space-padded blob to NVS
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    memset(tmp, 0x20, DEVICE_NUM_NAME_BYTES); // Initialize with spaces

    size_t copyLen = std::min(name.length(), static_cast<size_t>(DEVICE_NUM_NAME_BYTES));
    memcpy(tmp, name.c_str(), copyLen);
    err = WriteNvsBlob("Device_name", tmp, DEVICE_NUM_NAME_BYTES);
    //Serial.printf("[NVS] SetDeviceName '%s': %s\n", name.c_str(), esp_err_to_name(err));
}

uint16_t LocalStorage::GetDeviceBoolean(){
    uint8_t tmp[2];
    // Default to 0 (no flags set) if not found
    tmp[0] = 0;
    tmp[1] = 0;
    esp_err_t r = ReadNvsBlob("Device_bool", tmp, 2);
    if (r == ESP_ERR_NVS_NOT_FOUND) {
        return 0;
    }
    uint16_t boolValue = (static_cast<uint16_t>(tmp[0]) << 7) | static_cast<uint16_t>(tmp[1]);
    return boolValue;
}

void LocalStorage::SetDeviceBoolean(uint16_t deviceBoolean){
    uint8_t tmp[2];
    tmp[0] = static_cast<uint8_t>((deviceBoolean >> 7) & 0x7F);
    tmp[1] = static_cast<uint8_t>((deviceBoolean >> 0) & 0x7F);
    WriteNvsBlob("Device_bool", tmp, 2);
}

uint8_t LocalStorage::GetNumOfDistributors(){
    uint8_t num = ReadNvsU8("Distributor_num", 0);
    //Serial.printf("[NVS] GetNumOfDistributors: %d\n", num);
    return num;
}

void LocalStorage::SetNumOfDistributors(uint8_t numOfDistributors){
    WriteNvsU8("Distributor_num", numOfDistributors);
    //Serial.printf("[NVS] SetNumOfDistributors: %d\n", numOfDistributors);
}

void LocalStorage::GetDistributorConstruct(uint16_t distributorNum, uint8_t* construct){
    std::string key = Uint16ToKey(distributorNum);
    const char *ptr_key = key.c_str();
    err = ReadNvsBlob(ptr_key, construct, DISTRIBUTOR_NUM_CFG_BYTES);
    //if (err != ESP_OK) printf("Error (%s) saving run time blob to NVS!\n", esp_err_to_name(err));
}

void LocalStorage::SetDistributorConstruct(uint16_t distributorNum, const uint8_t* construct){
    std::string key = Uint16ToKey(distributorNum);
    const char *ptr_key = key.c_str();
    WriteNvsBlob(ptr_key, construct , DISTRIBUTOR_NUM_CFG_BYTES);
}

// Persist/Load Device ID as a 2-byte blob (MSB, LSB)
uint16_t LocalStorage::GetDeviceID(){
    uint8_t tmp[2];
    // Default to compile-time SYSEX_DEV_ID if not found
    tmp[0] = static_cast<uint8_t>((Device::GetDeviceID() >> 7) & 0x7F);
    tmp[1] = static_cast<uint8_t>((Device::GetDeviceID() >> 0) & 0x7F);
    esp_err_t r = ReadNvsBlob("Device_id", tmp, 2);
    if (r == ESP_ERR_NVS_NOT_FOUND) {
        return Device::GetDeviceID();
    }
    uint16_t id = (static_cast<uint16_t>(tmp[0]) << 7) | static_cast<uint16_t>(tmp[1]);
    return id;
}

void LocalStorage::SetDeviceID(uint16_t id){
    uint8_t tmp[2];
    tmp[0] = static_cast<uint8_t>((id >> 7) & 0x7F);
    tmp[1] = static_cast<uint8_t>((id >> 0) & 0x7F);
    WriteNvsBlob("Device_id", tmp, 2);
}

// Get key from valueHelper
std::string LocalStorage::Uint16ToKey(uint16_t value){
    const char hex[] = "0123456789abcdef";
    std::string key = "Distributor";
    key.push_back(hex[(value >> 0) & 0x0F]);
    key.push_back(hex[(value >> 4) & 0x0F]);
    key.push_back(hex[(value >> 8) & 0x0F]);
    key.push_back(hex[(value >> 12) & 0x0F]);

    return(key);
}

bool LocalStorage::InitializeDeviceConfiguration(DistributorManager& distributorManager){
    // Ensure NVS is properly initialized
    if (!EnsureNVSInitialized()) {
        return false; // NVS initialization failed, use defaults
    }

    // Device Config - only load if NVS is working
    Device::Name = GetDeviceName();
    // Load runtime device ID (if stored) and update Device runtime value
    Device::SetDeviceID(GetDeviceID());
    // Load device boolean flags (Muted, OmniMode)
    uint16_t deviceBoolValue = GetDeviceBoolean();
    Device::Muted = ((deviceBoolValue & DEVICE_BOOL_MASK::MUTED) != 0);
    Device::OmniMode = ((deviceBoolValue & DEVICE_BOOL_MASK::OMNIMODE) != 0);
    
    // Distributor Config
    uint8_t numDistributors = GetNumOfDistributors();
    for(uint8_t i = 0; i < numDistributors; i++){
        uint8_t distributorData[DISTRIBUTOR_NUM_CFG_BYTES];
        GetDistributorConstruct(i, distributorData);
        distributorManager.addDistributor(distributorData);
    }
    
    return true; // Successfully loaded configuration
}

#endif