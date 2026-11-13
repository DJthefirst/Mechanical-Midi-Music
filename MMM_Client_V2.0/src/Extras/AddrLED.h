#pragma once
#include "Config.h"

#ifdef CFG_EXTRA_ADDRESSABLE_LEDS
#include <FastLED.h>
#include <array>
#include <bitset>

#ifndef CFG_UPDATES_PER_SECOND
    #define CFG_UPDATES_PER_SECOND 100
#endif
static_assert(CFG_UPDATES_PER_SECOND > 0, "CFG_UPDATES_PER_SECOND must be greater than zero");

//TODO make configurable from GUI.
enum class HUE_METHOD{
    NOTE = 0,
    PITCH,
    ID,
    CHANNEL
};

class AddrLED{
    private:
        std::array<CRGB, CFG_NUM_LEDS> leds;
        HUE_METHOD m_hueMethod = HUE_METHOD::CHANNEL;
        bool m_hueTimeProgression = true;
        bool m_brightnessByVelocity = true;
        bool m_updateLEDs = false;

        // FastLED.show() blocks the main loop for about 30us per LED, which delays note
        // handling and solenoid timeouts. Refreshes are therefore batched and rate limited.
        static constexpr uint32_t REFRESH_INTERVAL_MS = 1000 / CFG_UPDATES_PER_SECOND;
        static constexpr uint32_t SETTLE_MS = 2; // Allows concurent notes processed sequentualy to udpate LEDs concurently 
        uint32_t m_lastShowMs = 0;
        uint32_t m_firstChangeMs = 0;

        // Keeps short flashes visible: an LED switched off before it was ever displayed
        // stays lit for one refresh and is cleared on the next.
        std::bitset<CFG_NUM_LEDS> m_notYetShown;
        std::bitset<CFG_NUM_LEDS> m_pendingOff;

        void markChanged();

    public:
        //Setup LEDs
        void setup();
        //Update LEDs
        void update();
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