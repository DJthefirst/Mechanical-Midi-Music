#pragma once
#include "Device.h"

#ifdef EXTRA_LOCAL_STORAGE

#include "nvs_flash.h"
#include "nvs.h"
#include <cstdint>
#include <string>
using std::int8_t;

class LocalStorage{
private:

    bool OpenNvs();
    std::string Uint16ToKey(uint16_t value);
    esp_err_t ReadNvsBlob(const char *key, uint8_t* result, uint8_t arrayLength);
    uint8_t ReadNvsU8(const char *key, uint8_t defaultValue);
    esp_err_t WriteNvsBlob(const char *key, const uint8_t* data, uint8_t arrayLength);
    void WriteNvsU8(const char *key, uint8_t value);

public:
    void Test();

    void ResetDeviceConfig();
    void GetDeviceConstruct();
    void SetDeviceConstruct();
    const char* GetDeviceName(uint8_t* data);
    void SetDeviceName(const char* name);
    uint8_t GetDeviceBoolean();
    void SetDeviceBoolean(uint8_t deviceBoolean);
    uint8_t GetNumOfDistributors();
    void SetNumOfDistributors(uint8_t numOfDistributors);
    void GetDistributorConstruct(uint16_t distributorNum, uint8_t* construct);
    void SetDistributorConstruct(uint16_t distributorNum, const uint8_t* construct);

    //Singleton
    LocalStorage(const LocalStorage&) = delete;
    static LocalStorage& get(){
        static LocalStorage localStorage;
        return localStorage;
    }

private:

    //Singleton Constructor
    LocalStorage();

}; 

#endif