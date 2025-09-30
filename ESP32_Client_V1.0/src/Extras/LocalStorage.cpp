//TODO: Add Error handling

#include "Device.h"
#ifdef EXTRA_LOCAL_STORAGE

#include <iostream>
#include <string>
#include "LocalStorage.h"
#include "Distributor.h"


esp_err_t err;
nvs_handle_t handle;


//Requires Serial.begin before use
LocalStorage::LocalStorage(){
    // Initialize NVS
    err = nvs_flash_init();
    //if (err != ESP_OK) Serial.printf("Error (%s) Init NVS!\n", esp_err_to_name(err));
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        //if (err != ESP_OK) Serial.printf("Error (%s) Init NVS!\n", esp_err_to_name(err));
    }
    ESP_ERROR_CHECK( err );
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
    esp_err_t err = nvs_open("test", NVS_READONLY, &testHandle);
    
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
        // NVS is not initialized, initialize it
        err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated and needs to be erased
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        if (err != ESP_OK) {
            return false;
        }
    } else if (err == ESP_OK) {
        // NVS is working, close the test handle
        nvs_close(testHandle);
    }
    // If err is ESP_ERR_NVS_NOT_FOUND, that's normal - the "test" namespace doesn't exist
    
    return true;
}

void LocalStorage::ResetDeviceConfig(){
    nvs_flash_erase();
    // Reinitialize NVS after erasing
    err = nvs_flash_init();
    if (err != ESP_OK) {
        // Handle any initialization errors
        ESP_ERROR_CHECK(err);
    }
}

//TODO Use Smart Pointer?
std::string LocalStorage::GetDeviceName(){
    // Read a fixed-size, space-padded name blob from NVS and convert to std::string
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    // Initialize with null bytes to ensure trailing bytes are padded if stored blob is shorter
    memset(tmp, 0x00, DEVICE_NUM_NAME_BYTES);
    err = ReadNvsBlob("Device_name", tmp, DEVICE_NUM_NAME_BYTES);

    if (err == ESP_ERR_NVS_NOT_FOUND){
        return Device::Name;
    }

    // Convert to std::string and trim trailing spaces
    std::string name(reinterpret_cast<char*>(tmp), DEVICE_NUM_NAME_BYTES);
    size_t end = name.find_last_not_of(' ');
    if (end != std::string::npos) name.erase(end + 1);
    else name.clear();

    return name;
}

void LocalStorage::SetDeviceName(std::string name){ 
    // Write a fixed-size 20-byte space-padded blob to NVS
    uint8_t tmp[DEVICE_NUM_NAME_BYTES];
    memset(tmp, 0x20, DEVICE_NUM_NAME_BYTES); // Initialize with spaces

    size_t copyLen = std::min(name.length(), static_cast<size_t>(DEVICE_NUM_NAME_BYTES));
    memcpy(tmp, name.c_str(), copyLen);
    WriteNvsBlob("Device_name", tmp, DEVICE_NUM_NAME_BYTES);
}

uint8_t LocalStorage::GetDeviceBoolean(){
    return ReadNvsU8("Device_bool", 0);
}

void LocalStorage::SetDeviceBoolean(uint8_t deviceBoolean){
    WriteNvsU8("Device_bool", deviceBoolean);
}

uint8_t LocalStorage::GetNumOfDistributors(){
    return ReadNvsU8("Distributor_num", 0);
}

void LocalStorage::SetNumOfDistributors(uint8_t numOfDistributors){
    WriteNvsU8("Distributor_num", numOfDistributors);
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

#endif