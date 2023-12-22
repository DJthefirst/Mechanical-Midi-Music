export const SYSEX_RegExpMsg = /F07D7F7F((?:[0-7][0-9a-fA-F])*)F7/di;
export const SYSEX_RegExpEnd = /^(?:[0-9a-fA-F][0-9a-fA-F])*F7/di;

export const SYSEX_START = 'F07D';
export const SYSEX_END = 'F7';

export const SYSEX_DeviceReady = '00';
export const SYSEX_ResetDeviceCfg = '01';

export const SYSEX_GetDeviceConstructFull = '10'; // With Distributors
export const SYSEX_GetDeviceConstructOnly = '11'; // Without Distributors
export const SYSEX_GetDeviceName = '12';
export const SYSEX_GetDeviceBoolean = '13';

export const SYSEX_SetDeviceConstructFull = '20'; // With Distributors
export const SYSEX_SetDeviceConstructOnly = '21'; // Without Distributors
export const SYSEX_SetDeviceName = '22';
export const SYSEX_SetDeviceBoolean = '23';

export const SYSEX_RemoveDistributorID = '30';
export const SYSEX_GetNumDistributors = '31';
export const SYSEX_AddDistributor = '32';

export const SYSEX_GetDistributorID = '34';

export const SYSEX_RemoveAllDistributors = '40';
export const SYSEX_GetAllDistributors = '41';

export const SYSEX_SetDistributor = '44';
export const SYSEX_SetDistributorID_Channels = '45';
export const SYSEX_SetDistributorID_Instruments = '46';
export const SYSEX_SetDistributorID_DistributionMethod = '47';
export const SYSEX_SetDistributorID_Boolean = '48';
export const SYSEX_SetDistributorID_MinMaxNotes = '49';
export const SYSEX_SetDistributorID_NumPolyphonicNotes = '4A';

