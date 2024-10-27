using System;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace MMM_Device;

//Algorythmic Methods to Distribute Notes Amoungst Instruments.
public enum DistributionMethod
{
    StraightThrough = 0,    //Each Channel goes to the Instrument with a matching ID ex. Ch 10 -> Instrument 10
    RoundRobin,             //Distributes Notes in a circular manner.
    RoundRobinBalance,      //Distributes Notes in a circular manner (Balances Notes across Instruments).
    Ascending,              //Plays Note on lowest available Instrument (Balances Notes across Instruments).
    Descending,             //Plays Note on highest available Instrument (Balances Notes across Instruments).
    Stack                   //TODO Play Notes Polyphonicaly on lowest available Instrument until full.
};

/* Routes Midi Notes to various instrument groups via configurable algorithms. */
public class Distributor
{
    //Size of Distributor when convered to Byte array
    public const int NUM_CFG_BYTES = 16;

    const int BOOL_MUTED = 0x01;
    const int BOOL_DAMPERPEDAL = 0x02;
    const int BOOL_POLYPHONIC = 0x04;
    const int BOOL_NOTEOVERWRITE = 0x08;

    //Local Atributes
    public int CurrentChannel { get; set; } = 0;
    public int CurrentInstrument { get; set; } = 0;

    //Each Bit Represents an Enabled Channel/Instrument (limits max number of instruments to 32)
    public int Channels { get; set; } = 0; //Represents Enabled MIDI Channels
    public int Instruments { get; set; } = 0; //Represents Enabled Instruments

    //Settings
    public bool Muted { get; set; } = false;
    public bool DamperPedal { get; set; } = false;
    public bool Polyphonic { get; set; } = true;
    public bool NoteOverwrite { get; set; } = false;
    public int MinNote { get; set; } = 0;
    public int MaxNote { get; set; } = 127;
    public int NumPolyphonicNotes { get; set; } = 1;
    public DistributionMethod DistributionMethod { get; set; } = DistributionMethod.RoundRobinBalance;

    public Distributor(byte[] data)
    {
        Console.WriteLine("Added a Distributor");
        this.SetDistributor(data);
    }

    //Returns Distributor construct without ID
    public List<byte> ToSerial()
    {
        List<byte> distributorObj = new List<byte>();

        byte distributorBoolByte = 0;
        if (Muted) distributorBoolByte |= BOOL_MUTED;
        if (DamperPedal) distributorBoolByte |= BOOL_DAMPERPEDAL;
        if (Polyphonic) distributorBoolByte |= BOOL_POLYPHONIC;
        if (NoteOverwrite) distributorBoolByte |= BOOL_NOTEOVERWRITE;


        //Cast Distributor Construct to uint8_t Array every MSB = 0 as per the Midi Protocal
        // (https://docs.google.com/spreadsheets/d/1AgS2-iZVLSL0w_MafbeReRx4u_9m_e4OTCsIhKC-QMg/edit?usp=sharing)
        distributorObj[0] = 0; //Distributor ID MSB Generated in MsgHandler
        distributorObj[1] = 0; //Distributor ID LSB Generated in MsgHandler
        distributorObj[2] = (byte)((Channels >> 14) & 0x03);
        distributorObj[3] = (byte)((Channels >> 7) & 0x7F);
        distributorObj[4] = (byte)((Channels >> 0) & 0x7F);
        distributorObj[5] = (byte)((Instruments >> 28) & 0x0F);
        distributorObj[6] = (byte)((Instruments >> 21) & 0x7F);
        distributorObj[7] = (byte)((Instruments >> 14) & 0x7F);
        distributorObj[8] = (byte)((Instruments >> 7) & 0x7F);
        distributorObj[9] = (byte)((Instruments >> 0) & 0x7F);
        distributorObj[10] = (byte)(DistributionMethod);
        distributorObj[11] = distributorBoolByte;
        distributorObj[12] = (byte)MinNote;
        distributorObj[13] = (byte)MaxNote;
        distributorObj[14] = (byte)NumPolyphonicNotes;
        distributorObj[15] = 0; //Reserved

        return distributorObj;
    }

    //Configures Distributor from construct
    public void SetDistributor(byte[] data)
    {
        // Decode Distributor Construct
        int channels =
              (data[2] << 14)
            | (data[3] << 7)
            | (data[4] << 0);
        int instruments =
              (data[5] << 28)
            | (data[6] << 21)
            | (data[7] << 14)
            | (data[8] << 7)
            | (data[9] << 0);
        DistributionMethod distributionMethod = (DistributionMethod)(data[10]);

        Channels = channels; // 1
        Instruments = instruments; // 1,2
        DistributionMethod = distributionMethod;
        Muted = (data[11] & BOOL_MUTED) != 0;
        DamperPedal = (data[11] & BOOL_DAMPERPEDAL) != 0;
        Polyphonic = (data[11] & BOOL_POLYPHONIC) != 0;
        NoteOverwrite = (data[11] & BOOL_NOTEOVERWRITE) != 0;
        MinNote = data[12];
        MaxNote = data[13];
        NumPolyphonicNotes = (data[14]);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //Message Processor
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    //Determine Type of MIDI Msg and Call Associated Event
    //void ProcessMessage(MidiMessage message)
    //{
    //    m_currentChannel = message.channel();

    //    switch (message.type())
    //    {

    //        case (MIDI_NoteOff):
    //            noteOffEvent(message.buffer[1], message.buffer[2]);
    //            break;
    //        case (MIDI_NoteOn):
    //            noteOnEvent(message.buffer[1], message.buffer[2], message.channel());
    //            break;
    //        case (MIDI_KeyPressure):
    //            keyPressureEvent(message.buffer[1], message.buffer[2]);
    //            break;
    //        case (MIDI_ControlChange):
    //            controlChangeEvent(message.buffer[1], message.buffer[2]);
    //            break;
    //        case (MIDI_ProgramChange):

    //            programChangeEvent(message.buffer[1]);
    //            break;
    //        case (MIDI_ChannelPressure):
    //            channelPressureEvent(message.buffer[1]);
    //            break;
    //        case (MIDI_PitchBend):
    //            pitchBendEvent((message.buffer[1] << 7 | message.buffer[2]));
    //            break;
    //        case (MIDI_SysCommon):
    //            break;
    //    }
    //}

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    ////Event Handlers
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    ////Find the first Instrument playing the given Note and stop it.
    //void Distributor::noteOffEvent(uint8_t Note, uint8_t Velocity)
    //{
    //    uint8_t instrument = checkForNote(Note);
    //    if (instrument != NONE)
    //    {
    //        (*m_ptrInstrumentController).stopNote(instrument, Note, Velocity);
    //    }
    //}

    ////Get Next Instument Based on Distribution Method and Play Note
    //void Distributor::noteOnEvent(uint8_t Note, uint8_t Velocity, uint8_t channel)
    //{
    //    //Check if note has 0 velocity representing a note off event
    //    if ((Velocity == 0))
    //    {
    //        noteOffEvent(Note, Velocity);
    //        return;
    //    }

    //    //Check if distributor is muted
    //    if (m_muted) return;

    //    //Get next instrument based on distribution method and play note
    //    uint8_t instrument = nextInstrument();
    //    if (instrument != NONE)
    //    {
    //        (*m_ptrInstrumentController).playNote(instrument, Note, Velocity, channel);
    //    }
    //}

    ////Find the first active instrument playing said note and update its velocity
    //void Distributor::keyPressureEvent(uint8_t Note, uint8_t Velocity)
    //{
    //    uint8_t instrument = checkForNote(Note);
    //    if (instrument != NONE)
    //    {
    //        (*m_ptrInstrumentController).setKeyPressure(instrument, Note, Velocity);
    //    }
    //}

    //void Distributor::controlChangeEvent(uint8_t Controller, uint8_t Value)
    //{
    //    //Implemented in MessageHandler
    //}

    //void Distributor::programChangeEvent(uint8_t Program)
    //{
    //    //Not Yet Implemented
    //    (*m_ptrInstrumentController).resetAll();
    //}

    //void Distributor::channelPressureEvent(uint8_t Velocity)
    //{
    //    //Not Yet Implemented 
    //}

    ////Update Each Instruments Pitch Bend Value
    //void Distributor::pitchBendEvent(uint16_t pitchBend)
    //{
    //    for (uint8_t i = 0; i < MAX_NUM_INSTRUMENTS; i++)
    //    {
    //        if ((m_instruments & (1 << i)) != 0)
    //        {
    //            (*m_ptrInstrumentController).setPitchBend(i, pitchBend);
    //        }
    //    }
    //}

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    ////Helper Functions
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    //// Returns the instument ID of the next instrument to be played.
    //uint8_t Distributor::nextInstrument()
    //{
    //    bool validInsturment = false;
    //    uint8_t insturmentLeastActive = NONE;
    //    uint8_t leastActiveNotes = 255;

    //    switch (m_distributionMethod)
    //    {

    //        // Return the Instument ID matching the current Messages Channel ID
    //        case (DistributionMethod::StraightThrough):

    //            // m_currentInstrument = m_currentChannel;
    //            // if(!distributorHasInstrument(m_currentInstrument)) return m_currentInstrument;
    //            // return NONE;

    //            for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++)
    //            {
    //                // Increment current Instrument
    //                m_currentInstrument++;

    //                // Loop to first instrument if end is reached
    //                m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

    //                // Check if valid instrument
    //                if (!distributorHasInstrument(m_currentInstrument)) continue;

    //                // If there are no active notes this must be the least active Instrument return
    //                uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(m_currentInstrument);
    //                if (activeNotes == 0) return m_currentInstrument;

    //                // Set this to Least Active Instrument if instrumentLeastActive is not yet set.
    //                if (insturmentLeastActive == NONE)
    //                {
    //                    insturmentLeastActive = m_currentInstrument;
    //                    continue;
    //                }

    //                // Update the Least Active Instrument if needed.
    //                if (activeNotes < (*m_ptrInstrumentController).getNumActiveNotes(insturmentLeastActive))
    //                {
    //                    insturmentLeastActive = m_currentInstrument;
    //                }
    //            }
    //            return insturmentLeastActive;

    //        // Return the next instrument in the instrument pool
    //        case (DistributionMethod::RoundRobin):

    //            for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++)
    //            {
    //                m_currentInstrument++;

    //                //Loop to first instrument if end is reached
    //                m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

    //                //Check if valid instrument
    //                if (!distributorHasInstrument(m_currentInstrument)) continue;
    //                return m_currentInstrument;
    //            }
    //            return NONE;

    //        // Iterate through every instrument in the instrument pool starting at the current instrument
    //        // Return the first instument that has the lowest number of active notes.
    //        case (DistributionMethod::RoundRobinBalance):

    //            for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++)
    //            {
    //                // Increment current Instrument
    //                m_currentInstrument++;

    //                // Loop to first instrument if end is reached
    //                m_currentInstrument = (m_currentInstrument >= MAX_NUM_INSTRUMENTS) ? 0 : m_currentInstrument;

    //                // Check if valid instrument
    //                if (!distributorHasInstrument(m_currentInstrument)) continue;

    //                // If there are no active notes this must be the least active Instrument return
    //                uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(m_currentInstrument);
    //                if (activeNotes == 0) return m_currentInstrument;

    //                // Set this to Least Active Instrument if instrumentLeastActive is not yet set.
    //                if (insturmentLeastActive == NONE)
    //                {
    //                    insturmentLeastActive = m_currentInstrument;
    //                    continue;
    //                }

    //                // Update the Least Active Instrument if needed.
    //                if (activeNotes < (*m_ptrInstrumentController).getNumActiveNotes(insturmentLeastActive))
    //                {
    //                    insturmentLeastActive = m_currentInstrument;
    //                }
    //            }
    //            return insturmentLeastActive;

    //        // Return the least active instrument starting at the instrument 0
    //        case (DistributionMethod::Ascending):

    //            for (int i = 0; (i < MAX_NUM_INSTRUMENTS); i++)
    //            {

    //                // Check if valid instrument
    //                if (!distributorHasInstrument(i)) continue;
    //                validInsturment = true;

    //                // Check if instrument has the least active notes 
    //                uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

    //                // If there are no active notes this must be the least active instrument return
    //                if (activeNotes == 0) return i;

    //                // Update least active instrument
    //                if (activeNotes < leastActiveNotes)
    //                {
    //                    leastActiveNotes = activeNotes;
    //                    insturmentLeastActive = i;
    //                }
    //            }
    //            return validInsturment ? m_currentInstrument : NONE;

    //        // Return the least active instrument starting at the last instrument iterating in reverse.
    //        case (DistributionMethod::Descending):

    //            for (int i = (MAX_NUM_INSTRUMENTS - 1); (i >= 0); i--)
    //            {

    //                // Check if valid instrument
    //                if (!distributorHasInstrument(i)) continue;
    //                validInsturment = true;

    //                // Check if instrument has the least active notes 
    //                uint8_t activeNotes = (*m_ptrInstrumentController).getNumActiveNotes(i);

    //                // If there are no active notes this must be the least active Instrument return
    //                if (activeNotes == 0) return i;

    //                // Update least active instrument
    //                if (activeNotes < leastActiveNotes)
    //                {
    //                    leastActiveNotes = activeNotes;
    //                    insturmentLeastActive = i;
    //                }

    //            }
    //            return validInsturment ? m_currentInstrument : NONE;

    //        default:
    //            //TODO Handle Error
    //            return NONE;
    //    }
    //}

    ////Returns the instument which the first note is played or NONE.
    ////*Note this function is optimised for nonpolyphonic playback and the Distribution method.
    //uint8_t Distributor::checkForNote(uint8_t note)
    //{
    //    uint8_t instrument = m_currentInstrument;

    //    switch (m_distributionMethod)
    //    {

    //        // Check for note being played on the instrument in the current channel position
    //        case (DistributionMethod::StraightThrough):
    //            if ((*m_ptrInstrumentController).isNoteActive(m_currentChannel, note))
    //            {
    //                return m_currentChannel;
    //            }
    //            break;

    //        // Check for note being played on the instrument iterating backwards through all instruments
    //        case (DistributionMethod::RoundRobin):
    //        case (DistributionMethod::RoundRobinBalance):
    //            //Iterate through each instrument in reverse
    //            for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++)
    //            {
    //                //Decrement Instrument with Underflow Protection
    //                if (instrument == 0) instrument = MAX_NUM_INSTRUMENTS;
    //                instrument--;

    //                //Check if valid instrument
    //                if (!distributorHasInstrument(instrument)) continue;
    //                if ((*m_ptrInstrumentController).isNoteActive(instrument, note)) return instrument;
    //            }
    //            break;

    //        // Check for note being played on the instrument starting at instrument 0
    //        case (DistributionMethod::Ascending):
    //            for (int i = 0; i < MAX_NUM_INSTRUMENTS; i++)
    //            {

    //                //Check if valid instrument
    //                if (!distributorHasInstrument(i)) continue;
    //                if ((*m_ptrInstrumentController).isNoteActive(i, note)) return i;
    //            }
    //            break;

    //        // Check for note being played on the instrument starting at the last instrument iterating in reverse.
    //        case (DistributionMethod::Descending):
    //            for (int i = (MAX_NUM_INSTRUMENTS - 1); i >= 0; i--)
    //            {

    //                //Check if valid instrument
    //                if (!distributorHasInstrument(i)) continue;
    //                if ((*m_ptrInstrumentController).isNoteActive(i, note)) return i;
    //            }
    //            break;

    //        default:
    //            //TODO Handle Error
    //            break;
    //    }
    //    return NONE;
    //}

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    ////Helpers
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    ////Returns true if a distributor contains the designated instument in its pool.
    //bool Distributor::distributorHasInstrument(int instrumentId)
    //{
    //    return ((m_instruments & (1 << instrumentId)) != 0);
    //}
};