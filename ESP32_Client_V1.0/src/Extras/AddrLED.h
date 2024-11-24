#pragma once
#include "Device.h"

#ifdef EXTRA_ADDRESSABLE_LEDS
#include <FastLED.h>

//TODO make configurable from GUI.
enum class HUE_METHOD{
    NOTE = 0,
    PITCH,
    ID,
    CHANNEL
};

class AddrLED{
    private:
        std::array<CRGB, NUM_LEDS> leds;
        HUE_METHOD m_hueMethod = HUE_METHOD::CHANNEL;
        bool m_hueTimeProgression = true;
        bool m_brightnessByVelocity = true;

    public:
        //Setup LEDs
        void setup();
        //Generate LED color
        CHSV getColor(uint8_t id, uint8_t channel, uint8_t note, uint8_t velocity);
        //Set LED on by id
        void turnLedOn(uint8_t id,CHSV color);
        //Set an id LED to off
        void turnLedsOn(uint8_t idStart, uint8_t idEnd, CHSV color);
        //Set an id LED to off
        void turnLedOff(uint8_t id);
        //Reset Leds
        void reset();

        //Singleton
        AddrLED(const AddrLED&) = delete;
        AddrLED& operator=(const AddrLED&) = delete;

        static AddrLED& get(){
            static AddrLED addrLED;
            return addrLED;
        }

    private:
        //Singleton Constructor
        AddrLED(){};
};

#endif