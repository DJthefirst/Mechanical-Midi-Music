#include "AddrLed.h"

#ifdef EXTRA_ADDRESSABLE_LEDS

//Setup FAST LED
void AddrLED::setup(){
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
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
        case LED_HUE_METHOD::NOTE:
            hue = (note%25)*10; //Hue by Note
            break;
        case LED_HUE_METHOD::PITCH:
            hue = (note%128)*2; //Hue by Pitch
            break;
        case LED_HUE_METHOD::ID:
            hue = (id%2)*125; //Hue by id Position
            break;
        case LED_HUE_METHOD::CHANNEL:
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
void AddrLED::setLedOn(uint8_t id,CHSV color){
    leds[id] = color;
    FastLED.show();
}

//Set an id LED to off
void AddrLED::setLedOff(uint8_t id){
    leds[id] = CRGB::Black; 
    FastLED.show();
}

//Reset Leds
void AddrLED::reset(){
    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = CRGB::Black;
    }
    //FastLED.show();
}

#endif