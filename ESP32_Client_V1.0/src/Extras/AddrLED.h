#pragma once
#include "Device.h"

#ifdef EXTRA_ADDRESSABLE_LEDS
#include <FastLED.h>


enum LED_HUE_METHOD{
    HUE_BY_NOTE = 0,
    HUE_BY_PITCH,
    HUE_BY_ID,
    HUE_BY_CHANNEL
};

class AddrLED{
    private:
        CRGB leds[NUM_LEDS];
        LED_HUE_METHOD m_hueMethod = HUE_BY_CHANNEL;
        bool m_hueTimeProgression = true;
        bool m_brightnessByVelocity = true;

    public:
        //Setup LEDs
        void setup();
        //Generate LED color
        CHSV getColor(uint8_t id, uint8_t channel, uint8_t note, uint8_t velocity);
        //Set LED on by id
        void setLedOn(uint8_t id,CHSV color);
        //Set an id LED to off
        void setLedOff(uint8_t id);
        //Reset Leds
        void reset();

        //Singleton
        AddrLED(const AddrLED&) = delete;
        static AddrLED& get(){
            static AddrLED addrLed;
            return addrLed;
        }

    private:
        //Singleton Constructor
        AddrLED(){};
};

#endif