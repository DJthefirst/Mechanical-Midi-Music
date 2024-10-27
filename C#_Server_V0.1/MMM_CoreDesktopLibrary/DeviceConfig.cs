namespace MMM_CoreDesktopLibrary;

using MMM_Device;
using System;
using System.Reflection.Metadata;

//SYSEX Custom Protocal
public struct SysEx {

    public const int AddrBroadcast = 0; 
    public const int AddrController = 0x3FFF; //14 bit

    public const byte ManufacturerID = 0x7D; //Educational MIDI ID

    public const byte DeviceReady = 0x00;
    public const byte ResetDeviceConfig = 0x01;
    public const byte DiscoverDevices = 0x02;

    public const byte GetDeviceConstructWithDistributors = 0x10;
    public const byte GetDeviceConstruct = 0x11;
    public const byte GetDeviceName = 0x12;
    public const byte GetDeviceBoolean = 0x13;

    public const byte SetDeviceConstructWithDistributors = 0x20;
    public const byte SetDeviceConstruct = 0x21;
    public const byte SetDeviceName = 0x22;
    public const byte SetDeviceBoolean = 0x23;

    public const byte RemoveDistributor = 0x30;
    public const byte RemoveAllDistributors = 0x40;
    public const byte GetNumOfDistributors = 0x31;
    public const byte GetAllDistributors = 0x41;
    public const byte AddDistributor = 0x32;
    public const byte ToggleMuteDistributor = 0x42;

    public const byte GetDistributorConstruct = 0x34;
    public const byte GetDistributorChannels = 0x35;
    public const byte GetDistributorInstruments = 0x36;
    public const byte GetDistributorMethod = 0x37;
    public const byte GetDistributorBoolValues = 0x38;
    public const byte GetDistributorMinMaxNotes = 0x39;
    public const byte GetDistributorNumPolyphonicNotes = 0x3A;

    public const byte SetDistributor = 0x44;
    public const byte SetDistributorChannels = 0x45;
    public const byte SetDistributorInstruments = 0x46;
    public const byte SetDistributorMethod = 0x47;
    public const byte SetDistributorBoolValues = 0x48;
    public const byte SetDistributorMinMaxNotes = 0x49;
    public const byte SetDistributorNumPolyphonicNotes = 0x4A;
}

public class SysExMsg
{
    private int len;
    public byte[] buffer;
    public SysExMsg(byte[] buffer)
    {
        this.len = buffer.Count();
        bool validSysEx = ((len >= 7) && (buffer[0] == 0xF0) && (buffer[1] == SysEx.ManufacturerID));
        if (!validSysEx) { throw new ArgumentOutOfRangeException(); }
        this.buffer = buffer;
    }
    public int Source() { return (buffer[2] << 7) | buffer[3]; }
    public int Destination() {
        return (buffer[4] << 7) | buffer[5];}
    public byte Type() { return buffer[6]; }
    public byte[] Payload() { return buffer[7..]; }
    
}

internal class DeviceConfig
{
    Dictionary<int, Device> devices = new Dictionary<int, Device>();

    public DeviceConfig()
    {
        //Add Callback from MidiCore to MidiOutputs.
        Action<SysExMsg> action = new Action<SysExMsg>(
            (SysExMsg sysExMsg) =>
            {
                ProcessMessage(sysExMsg);
            }
        );
        MidiCore.AddSysExMsgCallBack(action);
    }

    private void ProcessMessage(SysExMsg msg)
    {

        //Check if Desitination Device ID is correct
        if (msg.Destination() != SysEx.AddrController) return;

        switch (msg.Type())
        {
            case SysEx.DeviceReady:
                SendMessage(msg.Source(), SysEx.GetDeviceConstruct);
                break;
            case SysEx.ResetDeviceConfig:
                SendMessage(msg.Source(), SysEx.ResetDeviceConfig);
                break;
            case SysEx.DiscoverDevices:
                SendMessage(SysEx.AddrBroadcast, SysEx.DiscoverDevices);
                break;
            case SysEx.GetDeviceConstructWithDistributors:
                break;
            case SysEx.GetDeviceConstruct:
                AddDevice(new Device(msg.Payload()));
                SendMessage(msg.Source(), SysEx.GetAllDistributors);
                break;
            case SysEx.GetDeviceName:
                devices[msg.Source()].Name = BitConverter.ToString(msg.Payload()[0..20]);
                break;
            case SysEx.GetDeviceBoolean:
                break;
            case SysEx.SetDeviceConstructWithDistributors:
                break;
            case SysEx.SetDeviceConstruct:
                break;
            case SysEx.SetDeviceName:
                break;
            case SysEx.SetDeviceBoolean:
                break;
            case SysEx.RemoveDistributor:
                break;
            case SysEx.RemoveAllDistributors:
                break;
            case SysEx.GetNumOfDistributors:
                for(byte i = 0; i < msg.Payload()[0]; ++i) SendMessage(msg.Source(), SysEx.GetDistributorConstruct, [i]);
                break;
            case SysEx.GetAllDistributors:
                devices[msg.Source()].SetAllDistributors(msg.Payload());
                break;
            case SysEx.AddDistributor:
                break;
            case SysEx.ToggleMuteDistributor:
                break;
            case SysEx.GetDistributorConstruct:
                devices[msg.Source()].Distributors.Add(new Distributor(msg.Payload()));
                break;
            case SysEx.GetDistributorChannels:
                break;
            case SysEx.GetDistributorInstruments:
                break;
            case SysEx.GetDistributorMethod:
                break;
            case SysEx.GetDistributorBoolValues:
                break;
            case SysEx.GetDistributorMinMaxNotes:
                break;
            case SysEx.GetDistributorNumPolyphonicNotes:
                break;
            case SysEx.SetDistributor: 
                break;
            case SysEx.SetDistributorChannels: 
                break;
            case SysEx.SetDistributorInstruments: 
                break;
            case SysEx.SetDistributorMethod: 
                break;
            case SysEx.SetDistributorBoolValues: 
                break;
            case SysEx.SetDistributorMinMaxNotes: 
                break;
            case SysEx.SetDistributorNumPolyphonicNotes: 
                break;


            default:
                Console.WriteLine("Debug Unkown SysEx");
                return;
        }
    }

    public static void SendMessage(int destinationID, byte msgType)
    { 
        SendMessage(destinationID, msgType, []); 
    }
        public static void SendMessage(int destinationID, byte msgType ,List<byte> payload)
    {
        byte[] header = [ 0xF0,
            SysEx.ManufacturerID,
            0x7F, //source Destination
            0x7F, //source Destination
            (byte)(destinationID >> 7) ,
            (byte)(destinationID & 0x7F),
            msgType];
        byte[] tail = [0xF7];
        byte[] buffer = header.Concat(payload).Concat(tail).ToArray();
        MidiCore.SendMsg(buffer, 0, buffer.Length, 0);
    }

    private void AddDevice(Device device)
    {
        if (devices.ContainsKey(device.SYSEX_DEV_ID))
        {
            devices[device.SYSEX_DEV_ID] = device;
        }
        else devices.Add(device.SYSEX_DEV_ID, device);
    }

    private void RemoveDevice(Device device)
    {
        devices.Remove(device.SYSEX_DEV_ID);
    }
}

