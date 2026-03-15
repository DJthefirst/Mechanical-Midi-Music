// Updated regex to handle both 7-bit safe and 8-bit bytes in payload
// Device ID and Command are still 7-bit safe, but payload may contain 8-bit bytes
// Use non-greedy matching (.*?) to stop at the first F7, not the last one
export const SYSEX_RegExpMsg = /F07D((?:[0-7][0-9a-fA-F]){2})7F7F((?:[0-7][0-9a-fA-F]){1})((?:[0-7][0-9a-fA-F])*)F7/di;
export const SYSEX_RegExpEnd = /^(?:[0-9a-fA-F][0-9a-fA-F])*F7/di;

// SysEx Protocol Constants
export const SYSEX_START = 'F07D';
export const SYSEX_END = 'F7';
export const SYSEX_ID = 0x7D; // Educational MIDI ID
export const SYSEX_BroadcastID = '0000'; // 0x0000 encoded in 14-bit
export const SYSEX_ServerID = '7F7F'; // 0x3FFF encoded in 14-bit (max uint14)

////////////////////////////////////////////////////////////////////////////////////////////////////
// System Commands (0x00 - 0x0F)
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_DeviceReady = '00';
export const SYSEX_ResetDeviceCfg = '01';
export const SYSEX_DiscoverDevices = '02';
export const SYSEX_Message = '03';
// 0x04 - 0x0E: Reserved
// 0x0F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// Reserved (0x10 - 0x1F)
////////////////////////////////////////////////////////////////////////////////////////////////////
// All reserved
// 0x1F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// Get/Set Device Commands (0x20 - 0x2F)
// Get: Send with no payload -> device responds with data
// Set: Send with payload -> device applies configuration
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_DeviceConstructFull = '20'; // Get/Set Device Construct with Distributors
export const SYSEX_DeviceConstructOnly = '21'; // Get/Set Device Construct w/o Distributors
export const SYSEX_DeviceID = '22';            // Get/Set Device ID
export const SYSEX_DeviceName = '23';          // Get/Set Device Name
export const SYSEX_DeviceBoolean = '24';       // Get/Set Device Boolean
// 0x25 - 0x2E: Reserved
// 0x2F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Management Commands (0x30 - 0x3F)
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_GetNumDistributors = '30';
export const SYSEX_AddSetDistributor = '31';       // Add/Set Distributor (payload: Distributor Construct)
export const SYSEX_GetAllDistributors = '32';
export const SYSEX_RemoveDistributorID = '33';
export const SYSEX_RemoveAllDistributors = '34';
export const SYSEX_ToggleMuteDistributorID = '35'; // Payload: DistributorID (2 Byte), Return: bool isMuted
// 0x36 - 0x3E: Reserved
// 0x3F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// Get/Set Distributor Commands (0x40 - 0x4F)
// Get: Send with Distributor ID -> device responds with data
// Set: Send with Distributor ID + data -> device applies configuration
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_DistributorID_Construct = '40';          // Get/Set Distributor (ID) Construct
export const SYSEX_DistributorID_Channels = '41';           // Get/Set Distributor (ID) Channels
export const SYSEX_DistributorID_Instruments = '42';        // Get/Set Distributor (ID) Instruments
export const SYSEX_DistributorID_DistributionMethod = '43'; // Get/Set Distributor (ID) Distribution Method
export const SYSEX_DistributorID_Boolean = '44';            // Get/Set Distributor (ID) Boolean Values
export const SYSEX_DistributorID_MinMaxNotes = '45';        // Get/Set Distributor (ID) Min/Max Note Values
export const SYSEX_DistributorID_NumPolyphonicNotes = '46'; // Get/Set Distributor (ID) Number of Polyphonic Notes
// 0x47 - 0x4E: Reserved
// 0x4F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// Instrument Commands (0x50 - 0x5F)
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_ResetAllInstruments = '50';        // Reset All Instruments (Set Sub Device Mode)
export const SYSEX_ResetInstrument = '51';             // Reset Instrument (ID)
export const SYSEX_GetInstrumentNumActiveNotes = '52'; // Get Instrument (ID) Num Active Notes
export const SYSEX_SetInstrumentDirectMessage = '53';  // Set Instrument (ID) Direct Message
export const SYSEX_SetInstrumentNoteOn = '54';         // Set Instrument (ID) Note On
export const SYSEX_SetInstrumentNoteOff = '55';        // Set Instrument (ID) Note Off
// 0x56 - 0x5E: Reserved
// 0x5F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom Commands (0x60 - 0x6F)
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_Storage = '60';     // Storage (Sub Command)
export const SYSEX_LED = '61';         // LED (Sub Command)
export const SYSEX_SDPlayback = '62';  // SD Playback (Sub Command)
// 0x63 - 0x6E: Reserved
// 0x6F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// User Commands (0x70 - 0x7F)
////////////////////////////////////////////////////////////////////////////////////////////////////
export const SYSEX_CustomCmd1 = '70';  // Custom CMD 1 (1 Byte)
export const SYSEX_CustomCmd2 = '71';  // Custom CMD 2 (1 Byte)
export const SYSEX_CustomCmd3 = '72';  // Custom CMD 3 (1 Byte)
export const SYSEX_CustomCmd4 = '73';  // Custom CMD 4 (1 Byte)
export const SYSEX_CustomCmd5 = '74';  // Custom CMD 5 (1 Byte)
export const SYSEX_CustomCmd6 = '75';  // Custom CMD 6 (1 Byte)
export const SYSEX_CustomCmd7 = '76';  // Custom CMD 7 (1 Byte)
export const SYSEX_CustomCmd8 = '77';  // Custom CMD 8 (1 Byte)
export const SYSEX_CustomCmd9 = '78';  // Custom CMD 9 (2 Byte)
export const SYSEX_CustomCmd10 = '79'; // Custom CMD 10 (2 Byte)
export const SYSEX_CustomCmd11 = '7A'; // Custom CMD 11 (2 Byte)
export const SYSEX_CustomCmd12 = '7B'; // Custom CMD 12 (2 Byte)
export const SYSEX_CustomCmd13 = '7C'; // Custom CMD 13 (4 Byte)
export const SYSEX_CustomCmd14 = '7D'; // Custom CMD 14 (4 Byte)
export const SYSEX_CustomCmd15 = '7E'; // Custom CMD 15 (8 Byte)
// 0x7F: Extend

////////////////////////////////////////////////////////////////////////////////////////////////////
// MIDI CC Constants
////////////////////////////////////////////////////////////////////////////////////////////////////
export const MidiCC = {
	BankSelect: 0,
	ModulationWheel: 1,
	BreathControl: 2,
	FootPedal: 4,
	PortamentoTime: 5,
	Volume: 7,
	Pan: 10,
	Expression: 11,
	EffectCrtl_1: 12,
	EffectCrtl_2: 13,
	DamperPedal: 64,
	Portamento: 65,
	Sostenuto: 66,
	SoftPedal: 67,
	Legato: 68,
	Hold2: 69
} as const;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////////////////////////

export enum Instrument {
	None = 0,
	SW_PWM = 1,
	HW_PWM = 2,
	StepHW_PWM = 3,
	StepSW_PWM = 4,
	ShiftRegister = 5,
	MultiPhase = 6,
	StepperMotor = 10,
	FloppyDrive = 11
}

export enum Platform {
	None = 0,
	ESP32 = 1,
	ESP8266 = 2,
	Teensy41 = 3,
	ArduinoUno = 4,
	ArduinoMega = 5,
	ArduinoDue = 6,
	ArduinoMicro = 7,
	ArduinoNano = 8
}

export enum DistributionMethod {
	StraightThrough = 0, // Each channel goes to the instrument with matching ID
	RoundRobin = 1, // Distributes notes in a circular manner
	RoundRobinBalance = 2, // Distributes notes in a circular manner (balances notes)
	Ascending = 3, // Plays note on lowest available instrument (balances notes)
	Descending = 4, // Plays note on highest available instrument (balances notes)
	Stack = 5 // Play notes polyphonically on lowest available instrument until full
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Device Constants
////////////////////////////////////////////////////////////////////////////////////////////////////
export const DEVICE_NUM_NAME_BYTES = 20;
export const DEVICE_NAME_OFFSET = 36;
export const DEVICE_NUM_CFG_BYTES = 56;

// Device Boolean bit masks
export const DEVICE_BOOL_MASK = {
	MUTED: 1 << 0,
	OMNIMODE: 1 << 1,
	DAMPER: 1 << 2,
	VIBRATO: 1 << 3
} as const;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Constants
////////////////////////////////////////////////////////////////////////////////////////////////////
export const DISTRIBUTOR_NUM_CFG_BYTES = 24;

// Distributor Boolean bit masks
export const DISTRIBUTOR_BOOL_MASK = {
	MUTED: 1 << 0,
	NOTEOVERWRITE: 1 << 1
} as const;

// Instrument type lookup (handles sparse enum values)
export const InstrumentTypeNames: Record<number, string> = {
	[Instrument.None]: 'N/A',
	[Instrument.SW_PWM]: 'SW PWM',
	[Instrument.HW_PWM]: 'HW PWM',
	[Instrument.StepHW_PWM]: 'Step HW PWM',
	[Instrument.StepSW_PWM]: 'Step SW PWM',
	[Instrument.ShiftRegister]: 'ShiftRegister',
	[Instrument.MultiPhase]: 'MultiPhase',
	[Instrument.StepperMotor]: 'Stepper Motor',
	[Instrument.FloppyDrive]: 'Floppy Drive'
};

// Platform type lookup
export const PlatformTypeNames: Record<number, string> = {
	[Platform.None]: 'N/A',
	[Platform.ESP32]: 'ESP32',
	[Platform.ESP8266]: 'ESP8266',
	[Platform.Teensy41]: 'Teensy 4.1',
	[Platform.ArduinoUno]: 'Arduino Uno',
	[Platform.ArduinoMega]: 'Arduino Mega',
	[Platform.ArduinoDue]: 'Arduino Due',
	[Platform.ArduinoMicro]: 'Arduino Micro',
	[Platform.ArduinoNano]: 'Arduino Nano'
};

