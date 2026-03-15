#include "AddrLED.h"

#ifdef CFG_EXTRA_ADDRESSABLE_LEDS

//Setup FAST LED
void AddrLED::setup(){
    FastLED.addLeds<CFG_LED_TYPE, CFG_PIN_LED_DATA, CFG_COLOR_ORDER>(leds.data(), CFG_NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(CFG_BRIGHTNESS);
}

//Update LEDs
void AddrLED::update(){
    if(m_updateLEDs){
        FastLED.show();
        m_updateLEDs = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//FAST LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

//Generate LED color
CHSV AddrLED::getColor(uint8_t id, uint8_t channel, uint8_t note, uint8_t velocity){
    uint8_t hue;
    static uint8_t offset = 0;
    offset++;

    //Set the color of the led
    switch(m_hueMethod){
        case HUE_METHOD::NOTE:
            hue = (note%25)*10; //Hue by Note
            break;
        case HUE_METHOD::PITCH:
            hue = (note%128)*2; //Hue by Pitch
            break;
        case HUE_METHOD::ID:
            hue = (id%2)*125; //Hue by id Position
            break;
        case HUE_METHOD::CHANNEL:
            hue = (channel%4)*64; //Hue by Channel & Time
            break;
    }

    //Increment color over time
    if(m_hueTimeProgression) hue += offset;

    //Set brightness from note velocity
    if(m_brightnessByVelocity) velocity = velocity*2;
    else velocity = 255;
    
    return CHSV(hue,255,velocity);
}

//Set LED on by id
void AddrLED::turnLedOn(uint8_t id,CHSV color){
    if (id >= CFG_NUM_LEDS) return;
    leds[id] = color;
    m_updateLEDs = true;
}

//Set LED on by id
void AddrLED::turnLedsOn(uint8_t idStart, uint8_t idEnd, CHSV color){
    if (idStart >= CFG_NUM_LEDS) return;
    if (idEnd >= CFG_NUM_LEDS) idEnd = CFG_NUM_LEDS - 1;
    if (idStart > idEnd) return;

    for(int i=idStart; i <= idEnd; i++)leds[i] = color;
    m_updateLEDs = true;
}

//Set an id LED to off
void AddrLED::turnLedOff(uint8_t id){
    if (id >= CFG_NUM_LEDS) return;
    leds[id] = CRGB::Black; 
    m_updateLEDs = true;
}

//Reset Leds
void AddrLED::reset(){
    for( int i = 0; i < CFG_NUM_LEDS; ++i) {
        leds[i] = CRGB::Black;
    }
    m_updateLEDs = true;
}

#endif