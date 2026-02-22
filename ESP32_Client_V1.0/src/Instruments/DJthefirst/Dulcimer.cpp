#include "Config.h"
#include "Extras/AddrLED.h"
#include "Instruments/DJthefirst/Dulcimer.h"
#include "Arduino.h"

#if defined(PLATFORM_TEENSY41) && defined(CFG_INSTRUMENT_DULCIMER) && defined(CFG_COMPONENT_SHIFTREGISTER)

// Static member definitions
IShiftRegister<NUM_REG1_OUTPUTS>* Dulcimer::m_shiftReg1 = nullptr;
IShiftRegister<NUM_REG2_OUTPUTS>* Dulcimer::m_shiftReg2 = nullptr;
uint8_t Dulcimer::m_numActiveNotes = 0;

Dulcimer::Dulcimer() 
{
    // Initialize tracking arrays
    m_noteStartTime.fill(0);
    m_numActiveNotes = 0;
    
    // Create shift register instances using factory
    m_shiftReg1 = ShiftRegisterFactory::create<NUM_REG1_OUTPUTS>(
        CFG_PINS_INSTRUMENT_ShiftRegister1);
    m_shiftReg2 = ShiftRegisterFactory::create<NUM_REG2_OUTPUTS>(
        CFG_PINS_INSTRUMENT_ShiftRegister2);
    
    // Initialize shift registers
    m_shiftReg1->init();
    m_shiftReg2->init();
    
    // Setup LEDs
    setupLEDs();
    
    delay(500); // Wait a half second for safety
}

void Dulcimer::periodic() {
    checkSolenoidTimeouts();
    updateLEDs();
}

void Dulcimer::checkSolenoidTimeouts() {
    uint32_t currentTime = millis();
    
    if (m_numActiveNotes == 0) return; // No active notes, skip checking
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
        // Check if this note is active
        if (m_noteStartTime[i] != 0) {
            // Check if the solenoid on-time has expired
            if (currentTime - m_noteStartTime[i] >= CFG_NOTE_TIMEOUT_MS) {

                // Turn off the appropriate solenoid
                if (i < NUM_REG1_OUTPUTS) {
                    m_shiftReg1->setOutputEnabled(i, false);
                } else if (i >= NUM_REG1_OUTPUTS && i < NUM_OUTPUTS) {
                    m_shiftReg2->setOutputEnabled(i - NUM_REG1_OUTPUTS, false);
                }
                m_numActiveNotes--;
                
                // Clear tracking
                m_noteStartTime[i] = 0;
                
                // Push Update
                m_shiftReg1->update();
                m_shiftReg2->update();
                setInstrumentLedOff(i);
            }
        }
    }
}

void Dulcimer::playNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    // Calculate note position
    uint8_t notePos = NOTE_TO_SHIFT_REG_OUTPUT[note];

    // Check if note is maped to an output
    if (NOTE_TO_SHIFT_REG_OUTPUT[note] == 0) return;
    notePos--; // Decrement notePos by 1 to convert from 1-based to 0-based indexing

    // Record when this note started
    m_noteStartTime[notePos] = millis();
    m_numActiveNotes++;
    
    // Turn on the appropriate solenoid
    if (notePos < NUM_REG1_OUTPUTS) {
        m_shiftReg1->setOutputEnabled(notePos, true);
    } else if (notePos >= NUM_REG1_OUTPUTS && notePos < NUM_OUTPUTS) {
        m_shiftReg2->setOutputEnabled(notePos - NUM_REG1_OUTPUTS, true);
    }
    
    // Push Update
    m_shiftReg1->update();
    m_shiftReg2->update();
    setInstrumentLedOn(notePos, channel, note, velocity);
}

void Dulcimer::stopNote(uint8_t instrument, uint8_t note, uint8_t velocity, uint8_t channel)
{
    // Calculate note position
    uint8_t notePos = NOTE_TO_SHIFT_REG_OUTPUT[note];

    // Check if note is maped to an output
    if (NOTE_TO_SHIFT_REG_OUTPUT[note] == 0) return;
    notePos--; // Decrement notePos by 1 to convert from 1-based to 0-based indexing
    
    // Turn off the appropriate solenoid
    if (notePos < NUM_REG1_OUTPUTS) {
        m_shiftReg1->setOutputEnabled(notePos, false);
    } else if (notePos >= NUM_REG1_OUTPUTS && notePos < NUM_OUTPUTS) {
        m_shiftReg2->setOutputEnabled(notePos - NUM_REG1_OUTPUTS, false);
    }
    
    // Clear tracking
    m_noteStartTime[notePos] = 0;
    m_numActiveNotes--;

    // Push Update
    m_shiftReg1->update();
    m_shiftReg2->update();
    setInstrumentLedOff(notePos);
}

void Dulcimer::reset(uint8_t instrument) {
    resetAll(); // No individual instrument reset, just reset everything
}

void Dulcimer::resetAll() {
    // Disable all outputs on both shift registers
    m_shiftReg1->disableAll();
    m_shiftReg2->disableAll();
    
    m_shiftReg1->update();
    m_shiftReg2->update();
    
    // Clear all tracking
    m_noteStartTime.fill(0);
    m_numActiveNotes = 0;
    
    // Reset LEDs
    resetLEDs();
}

void Dulcimer::stopAll() {
    resetAll();
}

uint8_t Dulcimer::getNumActiveNotes(uint8_t instrument) {
    return m_numActiveNotes;
}

bool Dulcimer::isNoteActive(uint8_t instrument, uint8_t note) {
    uint8_t notePos = NOTE_TO_SHIFT_REG_OUTPUT[note];
    if (notePos == 0) return false; // Note not mapped to an output
    return m_noteStartTime[notePos-1] != 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LED Helper Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CFG_EXTRA_ADDRESSABLE_LEDS

void Dulcimer::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) {
    CHSV color = AddrLED::get().getColor(instrument, channel, note, velocity);
    AddrLED::get().turnLedOn(instrument, color);
}

void Dulcimer::setInstrumentLedOff(uint8_t instrument) {
    AddrLED::get().turnLedOff(instrument);
}

#else
void Dulcimer::setInstrumentLedOn(uint8_t instrument, uint8_t channel, uint8_t note, uint8_t velocity) {}
void Dulcimer::setInstrumentLedOff(uint8_t instrument) {}
#endif

#endif // PLATFORM_TEENSY41 && CFG_INSTRUMENT_DULCIMER && CFG_COMPONENT_SHIFTREGISTER