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

export const SYSEX_DeviceReady = '00';
export const SYSEX_ResetDeviceCfg = '01';
export const SYSEX_DiscoverDevices = '02';
export const SYSEX_Message = '03';
export const SYSEX_DeviceChanged = '04';

export const SYSEX_GetDeviceConstructFull = '10'; // With Distributors
export const SYSEX_GetDeviceConstructOnly = '11'; // Without Distributors
export const SYSEX_GetDeviceID = '12';
export const SYSEX_GetDeviceName = '13';
export const SYSEX_GetDeviceBoolean = '14';

export const SYSEX_SetDeviceConstructFull = '20'; // With Distributors
export const SYSEX_SetDeviceConstructOnly = '21'; // Without Distributors
export const SYSEX_SetDeviceID = '22';
export const SYSEX_SetDeviceName = '23';
export const SYSEX_SetDeviceBoolean = '24';

export const SYSEX_GetNumDistributors = '30';
export const SYSEX_AddDistributor = '31';
export const SYSEX_GetAllDistributors = '32';
export const SYSEX_RemoveDistributorID = '33';
export const SYSEX_RemoveAllDistributors = '34';
export const SYSEX_ToggleMuteDistributorID = '35';

export const SYSEX_GetDistributorID = '40';
export const SYSEX_GetDistributorID_Channels = '41';
export const SYSEX_GetDistributorID_Instruments = '42';
export const SYSEX_GetDistributorID_DistributionMethod = '43';
export const SYSEX_GetDistributorID_Boolean = '44';
export const SYSEX_GetDistributorID_MinMaxNotes = '45';
export const SYSEX_GetDistributorID_NumPolyphonicNotes = '46';

export const SYSEX_SetDistributor = '50';
export const SYSEX_SetDistributorID_Channels = '51';
export const SYSEX_SetDistributorID_Instruments = '52';
export const SYSEX_SetDistributorID_DistributionMethod = '53';
export const SYSEX_SetDistributorID_Boolean = '54';
export const SYSEX_SetDistributorID_MinMaxNotes = '55';
export const SYSEX_SetDistributorID_NumPolyphonicNotes = '56';

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
	StepperMotor = 0x10,
	FloppyDrive = 0x11
}

export enum Platform {
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
export const DEVICE_NAME_OFFSET = 20;
export const DEVICE_NUM_CFG_BYTES = 40;

// Device Boolean bit masks
export const DEVICE_BOOL_MASK = {
	MUTED: 1 << 0,
	OMNIMODE: 1 << 1
} as const;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Distributor Constants
////////////////////////////////////////////////////////////////////////////////////////////////////
export const DISTRIBUTOR_NUM_CFG_BYTES = 24;

// Distributor Boolean bit masks
export const DISTRIBUTOR_BOOL_MASK = {
	MUTED: 1 << 0,
	POLYPHONIC: 1 << 1,
	NOTEOVERWRITE: 1 << 2,
	DAMPERPEDAL: 1 << 3,
	VIBRATO: 1 << 4
} as const;

