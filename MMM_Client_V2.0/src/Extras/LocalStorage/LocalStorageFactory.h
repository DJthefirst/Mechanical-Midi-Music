/*
 * LocalStorageFactory.h
 * Factory for creating platform-specific local storage instances
 */

#pragma once

#include "Config.h"
#include "ILocalStorage.h"
#include <memory>

#if defined(PLATFORM_ESP32)
    #include "ESP32LocalStorage.h"
#elif defined(PLATFORM_TEENSY41)
    #include "Teensy41LocalStorage.h"
#else
    #include "NullLocalStorage.h"
#endif

/**
 * @brief Factory class for creating platform-specific local storage instances
 * 
 * This factory uses compile-time platform detection to instantiate the appropriate
 * storage implementation for the target platform.
 */
class LocalStorageFactory {
public:
    /**
     * @brief Create a platform-specific local storage instance
     * @return Unique pointer to ILocalStorage implementation
     */
    static std::unique_ptr<ILocalStorage> create() {
        #if defined(PLATFORM_ESP32)
            return std::make_unique<ESP32LocalStorage>();
        #elif defined(PLATFORM_TEENSY41)
            return std::make_unique<Teensy41LocalStorage>();
        #else
            return std::make_unique<NullLocalStorage>();
        #endif
    }

    /**
     * @brief Get singleton instance of local storage
     * @return Reference to the local storage singleton
     */
    static ILocalStorage& getInstance() {
        static std::unique_ptr<ILocalStorage> instance = create();
        if (instance) {
            // Initialize on first access
            static bool initialized = false;
            if (!initialized) {
                instance->initialize();
                initialized = true;
            }
        }
        return *instance;
    }

    // Delete copy and move constructors
    LocalStorageFactory() = delete;
    LocalStorageFactory(const LocalStorageFactory&) = delete;
    LocalStorageFactory& operator=(const LocalStorageFactory&) = delete;
    LocalStorageFactory(LocalStorageFactory&&) = delete;
    LocalStorageFactory& operator=(LocalStorageFactory&&) = delete;
};
