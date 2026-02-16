#pragma once

#include <bitset>
#include <cstdint>

template<size_t numOutputs>
class IShiftRegister {
protected:
    bool m_initialized = false;
    bool m_update = false;
    std::bitset<numOutputs> m_outputEnabled = {};

    uint8_t m_PIN_SER;
    uint8_t m_PIN_CLK;
    uint8_t m_PIN_LD;
    uint8_t m_PIN_EN;
    uint8_t m_PIN_RST;

public:
    IShiftRegister(uint8_t PIN_SER, uint8_t PIN_CLK, uint8_t PIN_LD, uint8_t PIN_EN, uint8_t PIN_RST):
        m_PIN_SER(PIN_SER), m_PIN_CLK(PIN_CLK), m_PIN_LD(PIN_LD), m_PIN_EN(PIN_EN), m_PIN_RST(PIN_RST){}
    virtual void init() = 0;
    virtual void setOutputEnabled(uint8_t output, bool enabled) = 0;
    virtual bool getOutputEnabled(uint8_t output) = 0;
    virtual void disableAll() = 0;
    virtual void update() = 0;
};