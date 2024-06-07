using Commons.Music.Midi;
using CoreMidi;
using System.IO.Ports;
using System.Reflection;

namespace MMM_CoreDesktopLibrary;

internal class SerialIO : INetworkOut, INetworkIn
{
    private static List<SerialPort> SerialPorts = new List<SerialPort>();

    public SerialIO(){

        //Add Callback from MidiCore to MidiOutputs.
        Action<byte[], int, int, long> action = new Action<byte[], int, int, long>(
            (byte[] mevent, int offset, int length, long timestamp) =>
            {
                foreach (var port in SerialPorts)
                {
                    port.Write(mevent, offset, length);
                }
            }
        );
        MidiCore.AddMidiMsgCallBack(action);
    }

    public List<String> GetAvailable()
    {
        return SerialPort.GetPortNames().ToList();
    }
    public List<String> ListConnected()
    {
        return SerialPorts.ConvertAll(port => (port.PortName));
    }
    public void Add(string portName)
    {
        if (!GetAvailable().Contains(portName)) { Console.WriteLine("Invalid port name."); return; }
        if (ListConnected().Contains(portName)){ Console.WriteLine("Port already connected."); return; }

        string[] ports = SerialPort.GetPortNames();
        if (ports.Contains(portName))
        {
            SerialPort port = new SerialPort(portName);
            try
            {

                port.DataReceived += new SerialDataReceivedEventHandler(MessageReceived);
                port.BaudRate = 115200;
                port.Open();
                SerialPorts.Add(port);
                DeviceConfig.SendMessage(SysEx.AddrBroadcast, SysEx.GetDeviceConstruct);
                Console.WriteLine(portName + " Connected.");
            }
            catch
            {
                Console.WriteLine("Failed to open port.");
            }
        }
        else Console.WriteLine("Port not found.");
    }

    public void Remove(string portName)
    {
        if (portName == null){ Console.WriteLine("Invalid port name."); return; }
        int idx = ListConnected().FindIndex(0, SerialPorts.Count, (name => (name == portName)));
        if (idx == -1)
        {
            Console.WriteLine("Port not found.");
        }
        else
        {
            SerialPorts[idx].Close();
            SerialPorts.RemoveAt(idx);
            Console.WriteLine(portName + " Disconnected.");
        }
    }

    public void Clear()
    {
        foreach (SerialPort port in SerialPorts) port.Close();
        SerialPorts.Clear();
    }

    private void MessageReceived(object sender, SerialDataReceivedEventArgs e)
    {
        SerialPort port = (SerialPort)sender;
        int len = port.BytesToRead;
        byte[] buffer = new byte[len];
        port.Read(buffer, 0, len);
        // Filter Out nonMidi Messages.
        try { foreach (var midiEvent in MidiEvent.Convert(buffer, 0, len)); } catch { return; }

        MidiCore.SendMsg(buffer, 0, len, 0);
        
    }
}

