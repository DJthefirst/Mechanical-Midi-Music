// #include <FastLED.h>

// //FAST LED Variables
// #define LED_PIN     33
// #define NUM_LEDS    8
// #define BRIGHTNESS  255
// #define LED_TYPE    WS2811
// #define COLOR_ORDER GRB
// CRGB leds[NUM_LEDS];

// #define UPDATES_PER_SECOND 100

// //Setup FAST LED
// FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
// FastLED.setBrightness(  BRIGHTNESS );

// ////////////////////////////////////////////////////////////////////////////////////////////////////
// //FAST LED Helper Functions
// ////////////////////////////////////////////////////////////////////////////////////////////////////

// //Set an Instrument Led to on
// void PwmDriver::setInstumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity){
//     static uint8_t offset = 0;
//     offset++;
//     //uint8_t hue = (note%25)*10; //Hue by Note
//     //uint8_t hue = (note%128)*2; //Hue by Pitch
//     //uint8_t hue = (instrument%2)*125; //Hue by Instrument Position
//     uint8_t hue = (channel%4)*64+offset; //Hue by Instrument Position
//     leds[instrument] = CHSV(hue,255,velocity*2); //Brightness by velocity
//     //leds[instrument] = CHSV(hue,255,255); 
//     FastLED.show();
// }

// //Set an Instrument Led to off
// void PwmDriver::setInstumentLedOff(uint8_t instrument){
//     leds[instrument] = CRGB::Black; 
//     FastLED.show();
// }

// //Reset Leds
// void PwmDriver::resetLEDs(){
//     for( int i = 0; i < NUM_LEDS; ++i) {
//         leds[i] = CRGB::Black;
//     }
//     // FastLED.show();
// }