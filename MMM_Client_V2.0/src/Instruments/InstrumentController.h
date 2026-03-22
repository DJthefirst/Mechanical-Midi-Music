#pragma once

#include "InstrumentControllerBase.h"
#include <memory>

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