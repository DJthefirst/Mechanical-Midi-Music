using System;

namespace MMM_Device
{
    public enum InstrumentType
    {
        INSTRUMENT_PWM = 1,
        INSTRUMENT_ShiftRegister,
        INSTRUMENT_StepperMotor,
        INSTRUMENT_FloppyDrive
    };

    public enum PlatformType
    {
        PLATFORM_ESP32 = 1,
        PLATFORM_ESP8266,
        PLATFORM_ArduinoUno,
        PLATFORM_ArduinoMega,
        PLATFORM_ArduinoDue,
        PLATFORM_ArduinoMicro,
        PLATFORM_ArduinoNano
    };

    public class Device
    {
        //Device Construct Constants
        const byte NUM_NAME_BYTES = 20;
        const byte NUM_CFG_BYTES = 30;
        const byte BOOL_OMNIMODE = 0x01;

        public Device(){}
        public Device(byte[] deviceConstruct) 
        { 
            this.SetDeviceConstruct(deviceConstruct);       
        }


        public string Name { get; set; } = "New Device";
        public bool OmniMode { get; set; } = false;

        //Interupt frequency. A smaller resolution produces more accurate notes but leads to instability.
        public int TIMER_RESOLUTION = 8;

        //---------- Device Configuration ----------

        //Instrument type
        public InstrumentType INSTRUMENT_TYPE { get; set; } = InstrumentType.INSTRUMENT_PWM;
        //Platform type
        public PlatformType PLATFORM_TYPE { get; set; } = PlatformType.PLATFORM_ESP32;
        //Absolute max number of Polyphonic notes is 16
        public byte MAX_POLYPHONIC_NOTES { get; set; } = 1;
        //Absolute max number of Instruments is 32
        public byte MAX_NUM_INSTRUMENTS { get; set; } = 1;
        //Absolute Lowest Note Min=0
        public byte MIN_MIDI_NOTE { get; set; } = 0;
        //Absolute Highest Note Max=127
        public byte MAX_MIDI_NOTE { get; set; } = 127;
        //A 14 bit number Representing this Devices ID
        public int SYSEX_DEV_ID { get; set; } = 0x0001;
        //Firmware Version 14bit
        public int FIRMWARE_VERSION { get; set; } = 0;

        public List<Distributor> Distributors = new List<Distributor>();


        private void SetDeviceBoolean(byte deviceBoolByte)
        {
            OmniMode = ((deviceBoolByte & 0x01) != 0);
        }

        public void SetDeviceConstruct(byte[] deviceObj)
        {
            SYSEX_DEV_ID = (deviceObj[0] << 7) | (deviceObj[1] << 0);
            SetDeviceBoolean(deviceObj[2]);
            MAX_NUM_INSTRUMENTS = deviceObj[3];
            INSTRUMENT_TYPE = (InstrumentType)deviceObj[4];
            PLATFORM_TYPE = (PlatformType)deviceObj[5];
            MIN_MIDI_NOTE = deviceObj[6];
            MAX_MIDI_NOTE = deviceObj[7];
            FIRMWARE_VERSION = (deviceObj[8] << 7) | (deviceObj[9] << 0);

            //Name = BitConverter.ToString(deviceObj[10..20]);
        }

        public void SetAllDistributors(byte[] data)
        {
            int numDistributors = (data.Length / Distributor.NUM_CFG_BYTES);
            Distributors.Clear();
            for (int i = 0; i < numDistributors; ++i)
            {
                int idx = i * Distributor.NUM_CFG_BYTES;
                Distributors.Add(new Distributor(data[(idx)..(idx+Distributor.NUM_CFG_BYTES)]));
            }
        }

        private byte GetDeviceBoolean()
        {
            byte deviceBoolByte = 0;
            if (OmniMode) deviceBoolByte |= (1 << 0);
            return deviceBoolByte;
        }

        public List<byte> GetDeviceConstruct()
        {
            List<byte> deviceObj = new List<byte>();

            deviceObj[0] = (byte)((SYSEX_DEV_ID >> 7) & 0x7F); //Device ID MSB
            deviceObj[1] = (byte)((SYSEX_DEV_ID >> 0) & 0x7F); //Device ID LSB
            deviceObj[2] = (byte)GetDeviceBoolean();
            deviceObj[3] = (byte)MAX_NUM_INSTRUMENTS;
            deviceObj[4] = (byte)INSTRUMENT_TYPE;
            deviceObj[5] = (byte)PLATFORM_TYPE;
            deviceObj[6] = (byte)MIN_MIDI_NOTE;
            deviceObj[7] = (byte)MAX_MIDI_NOTE;
            deviceObj[8] = (byte)((FIRMWARE_VERSION >> 7) & 0x7F);
            deviceObj[9] = (byte)((FIRMWARE_VERSION >> 0) & 0x7F);

            for (int i = 0; i < 20; i++)
            {
                if (Name.Length > i) deviceObj[10 + i] = (byte)Name.ToCharArray()[i];
                else deviceObj[10 + i] = 0;
            }

            return deviceObj;
        }
    }
}
