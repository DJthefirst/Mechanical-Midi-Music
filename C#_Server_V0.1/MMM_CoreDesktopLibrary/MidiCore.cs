using Commons.Music.Midi;
using System;
using System.ComponentModel;

namespace MMM_CoreDesktopLibrary;

class MidiCore : IMidiOutput
{
    public static bool logMidiEvents { get; set; } = false;

    private static Task completed_task = Task.FromResult(result: false);
    private static Queue<byte> buffer = new Queue<byte>();

    private static List<Action<SysExMsg>> sysExMsgCallBacks = new List<Action<SysExMsg>>();
    private static List<Action<byte[], int, int, long>> midiMsgCallBacks = new List<Action<byte[], int, int, long>>();

    public IMidiPortDetails Details => CreateDetails();

    public MidiPortConnectionState Connection { get; private set; }

    IMidiPortDetails CreateDetails()
    {
        return new MMM_PortDetails("MMM_Core", "MMM_Core");
    }

    public Task CloseAsync()
    {
        return completed_task;
    }

    public void Dispose()
    {
    }

    public static MidiCore Instance { get; private set; }

    static MidiCore()
    {
        Instance = new MidiCore();
    }

    public void Send(byte[] mevent, int offset, int length, long timestamp) { SendMsg(mevent, offset, length, timestamp); }
    
    public static void SendMsg(byte[] mevent, int offset, int length, long timestamp)
    {

        if (logMidiEvents)
        {
            //string hexString = Convert.ToHexString(mevent[..length]);
            //hexString = string.Join(" ", Enumerable.Range(0, hexString.Length / 2).Select(i => hexString.Substring(i * 2, 2)));
            //Console.WriteLine(hexString); 
        }

        //Push messge to buffer
        foreach (var item in mevent)
        {
            buffer.Enqueue(item);
        }

        if (buffer.Peek() == 0xF0 && !buffer.Contains(0xF7)) return;

        string hexString = Convert.ToHexString(buffer.ToArray());
        hexString = string.Join(" ", Enumerable.Range(0, hexString.Length / 2).Select(i => hexString.Substring(i * 2, 2)));
        //Console.WriteLine(hexString);

        //Check for SysEx messages to this controller
        SysExMsg? sysExMsg = null;
        try
        {
            int endindex = (buffer.ToList().IndexOf(0xF7));
            sysExMsg = new SysExMsg(buffer.ToArray()[0..endindex]);

            for (int i = 0; i <= endindex; i++) buffer.Dequeue();


            if (sysExMsg.Destination() != 0x3FFF)
            {
                Console.WriteLine("Sent: " + hexString);
            }
            else
            {
                Console.WriteLine("Received: " + hexString);
                //sysExMsg = null;
            }


            if (sysExMsg.Destination() != 0x3FFF) throw new Exception();
            //Console.WriteLine("Sucess!");

        }
        catch { sysExMsg = null; }//Console.WriteLine("MSG FAIL!"); }

			//Filter out SysEx messages to this controller
			if (sysExMsg != null) {

            foreach (var action in sysExMsgCallBacks)
            {
                action(sysExMsg);
            }

            //string hexString = Convert.ToHexString(mevent[..length]);
            //hexString = string.Join(" ", Enumerable.Range(0, hexString.Length / 2).Select(i => hexString.Substring(i * 2, 2)));
            Console.WriteLine("\nSysEx: " + hexString);
            //Console.WriteLine("Filtered");
            return; //Maybe causes bug?
        }

        //Console.WriteLine("UnFiltered");

        //myactions.Add(new Action<byte[], int, int>((byte[] mevent, int offset, int length) => { Console.WriteLine("Action 1"); }));
        foreach (var action in midiMsgCallBacks)
            action(mevent, offset, length, timestamp);

    }
        public static void AddMidiMsgCallBack(Action<byte[], int, int, long> callBack)
    {
        midiMsgCallBacks.Add(callBack);
    }

    public static void AddSysExMsgCallBack(Action<SysExMsg> callBack)
    {
        sysExMsgCallBacks.Add(callBack);
    }
}

class MMM_PortDetails : IMidiPortDetails
{
    public string Id { get; set; }

    public string Manufacturer { get; set; }

    public string Name { get; set; }

    public string Version { get; set; }

    public MMM_PortDetails(string id, string name)
    {
        Id = id;
        Manufacturer = "Mechanical Midi Music";
        Name = name;
        Version = "1.0";
    }
}
