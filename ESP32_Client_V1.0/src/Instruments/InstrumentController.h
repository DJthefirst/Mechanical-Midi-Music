#pragma once

#include "InstrumentControllerBase.h"
#include "Device.h"
#include <cstdint>
#include <memory>
using std::int8_t;
using std::int16_t;

// Singleton template class for instrument controllers
template<typename ConcreteInstrumentType>
class InstrumentController {
public:
    static std::shared_ptr<InstrumentControllerBase> getInstance() {
        static std::shared_ptr<InstrumentControllerBase> instance = 
            std::shared_ptr<InstrumentControllerBase>(new ConcreteInstrumentType());
        return instance;
    }
    
private:
    InstrumentController() = default;
};