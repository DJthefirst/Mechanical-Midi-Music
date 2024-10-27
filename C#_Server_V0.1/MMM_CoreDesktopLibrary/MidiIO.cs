using Commons.Music.Midi;
using System.Diagnostics;

namespace MMM_CoreDesktopLibrary;

internal class MidiIO
{
    IMidiOutput midiCore;
    IMidiAccess access;
    List<IMidiInput> midiInputs;
    List<IMidiOutput> midiOutputs;

    public MidiIO(IMidiOutput midiCore)
    {
        access = MidiAccessManager.Default;
        midiInputs = new List<IMidiInput>();
        midiOutputs = new List<IMidiOutput>();

        this.midiCore = midiCore;

        //Add Callback from MidiCore to MidiOutputs.
        Action<byte[], int, int, long> action = new Action<byte[], int, int, long>(
            (byte[] mevent, int offset, int length, long timestamp) =>
            {
                foreach (var output in midiOutputs)
                {
                    output.Send(mevent, offset, length, timestamp);
                }
            }
        );
        MidiCore.AddMidiMsgCallBack(action);
    }

    public List<(string, string, string, string)> GetAvailableMidiIn()
    {
        return access.Inputs.ToList().ConvertAll(input => (input.Id, input.Name, input.Manufacturer, input.Version));
    }

    public List<(string, string, string, string)> GetAvailableMidiOut()
    {
        return access.Outputs.ToList().ConvertAll(output => (output.Id, output.Name, output.Manufacturer, output.Version));
    }

    public List<(string, string, string, string)> ListConnectedInputs()
    {
        return midiInputs.ConvertAll(input => (
            input.Details.Id,
            input.Details.Name,
            input.Details.Manufacturer,
            input.Details.Version));
    }

    public List<(string, string, string, string)> ListConnectedOutputs()
    {
        return midiOutputs.ConvertAll(output => (
            output.Details.Id,
            output.Details.Name,
            output.Details.Manufacturer,
            output.Details.Version));
    }

    public async void AddInput(string Id)
    {
        if (midiInputs.ConvertAll(input => input.Details.Id).Contains(Id)) return;
        if (!access.Inputs.ToList().ConvertAll(input => input.Id).Contains(Id)) return;
        try
        {
            IMidiInput inputPort = await access.OpenInputAsync(Id);
            inputPort.MessageReceived += InputPort_MessageReceived;
            midiInputs.Add(inputPort);
            return;
        }
        catch(Exception e) {
            Debug.WriteLine(e);
            return; 
        }
    }

    private void InputPort_MessageReceived(object? sender, MidiReceivedEventArgs e)
    {
        midiCore.Send(e.Data, e.Start, e.Length, e.Timestamp);
    }

    public async void AddOutput(string Id)
    {
        if (midiOutputs.ConvertAll(output => output.Details.Id).Contains(Id)) { Console.WriteLine("Port all ready added!"); return; }
        if (!access.Outputs.ToList().ConvertAll(output => output.Id).Contains(Id)) { Console.WriteLine("Port not found!"); return; }
        try
        {

            IMidiOutput outputPort = await access.OpenOutputAsync(Id);
            midiOutputs.Add(outputPort);
            Console.WriteLine(outputPort.Details.Name + " added.");
            return;
        }
        catch (Exception e)
        {
            Debug.WriteLine(e);
            return;
        }
    }

    public bool RemoveInput(string Id)
    {
        if (!midiInputs.ConvertAll(input => input.Details.Id).Contains(Id)) { Console.WriteLine("Port not found!"); return false; }
        try
        {
            IMidiInput inputPort = midiInputs.Find(input => input.Details.Id == Id);
            midiInputs.Remove(inputPort);
            inputPort.CloseAsync();
            Console.WriteLine(inputPort.Details.Name + " removed.");
            return true;
        }
        catch (Exception e)
        {
            Debug.WriteLine(e);
            return false;
        }
    }

    public bool RemoveOutput(string Id)
    {
        if (!midiOutputs.ConvertAll(output => output.Details.Id).Contains(Id)) { Console.WriteLine("Port not found!"); return false; }
        try
        {
            IMidiOutput outputPort = midiOutputs.Find(output => output.Details.Id == Id);
            midiOutputs.Remove(outputPort);
            outputPort.CloseAsync();
            Console.WriteLine(outputPort.Details.Name + " removed.");
            return true;
        }
        catch (Exception e)
        {
            Debug.WriteLine(e);
            return false;
        }
    }

    public void ClearInputs()
    {
        foreach (var input in midiInputs)
        {
            input.CloseAsync();
        }
        midiInputs.Clear();
    }

    public void ClearOutputs()
    {
        foreach (var output in midiOutputs)
        {
            output.CloseAsync();
        }
        midiOutputs.Clear();
    }
}