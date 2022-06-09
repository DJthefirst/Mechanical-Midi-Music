using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Commons.Music.Midi;
using MMM_Networking;

namespace MMM_MIDI_Controller
{
    public class MidiController
    {
        IMidiOutput output;
        IMidiInput input;
        MidiPlayer player;
        UDPServer server;


        public void Initialize(UDPServer s)
        {
            server = s;
            var access = MidiAccessManager.Default;
            output = access.OpenOutputAsync(access.Outputs.Last().Id).Result;
            input = access.OpenInputAsync(access.Inputs.Last().Id).Result;

            //output = access.Outputs.FirstOrDefault(o => o.Id == arg.Substring(9));
            //if (output == null)

                String midi_file = @"C:\Users\djber\Desktop\Pirate.mid";
            var music = MidiMusic.Read(System.IO.File.OpenRead(midi_file));
            player = new MidiPlayer(music, output);
            player.EventReceived += (MidiEvent e) =>
            {
                if (e.EventType == MidiEvent.Program)
                    System.Diagnostics.Debug.WriteLine($"Program changed: Channel:{e.Channel} Instrument:{e.Msb}");
            };
            input.MessageReceived += Input_MessageReceived;
        }

        private void Input_MessageReceived(object? sender, MidiReceivedEventArgs e)
        {
            server.SendToMulticast(e.Data);
        }

        public void Play()
        {
            player.Play();
        }

        public void Stop()
        {
            Console.WriteLine("Type [CR] to stop.");

        }

        public void PlayNotes(){
            // Custom Notes
                        output.Send(new byte[] {0xC0, GeneralMidi.Instruments.AcousticGrandPiano}, 0, 2, 0); // There are constant fields for each GM instrument
                        output.Send(new byte[] { MidiEvent.NoteOn, 0x40, 0x70 }, 0, 3, 0); // There are constant fields for each MIDI event
                        output.Send(new byte[] { MidiEvent.NoteOff, 0x40, 0x70 }, 0, 3, 0);
                        output.Send(new byte[] { MidiEvent.Program, 0x30 }, 0, 2, 0); // Strings Ensemble
                        output.Send(new byte[] { 0x90, 0x40, 0x70 }, 0, 3, 0);
                        output.Send(new byte[] { 0x80, 0x40, 0x70 }, 0, 3, 0);
                        output.CloseAsync();
        }
    }
}
