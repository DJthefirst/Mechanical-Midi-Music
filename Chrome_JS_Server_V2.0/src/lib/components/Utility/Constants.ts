export const SYSEX_RegExpMsg = /F07D7F7F((?:[0-7][0-9a-fA-F])*)F7/di;
export const SYSEX_RegExpEnd = /^(?:[0-9a-fA-F][0-9a-fA-F])*F7/di;

export const SYSEX_START = 'F07D';
export const SYSEX_END = 'F7';

export const SYSEX_ResetDeviceCfg = '00';
export const SYSEX_GetDeviceConstructFull = '01'; // With Distributors
export const SYSEX_GetDeviceConstructOnly = '02'; // Without Distributors
export const SYSEX_GetDeviceName = '03';
export const SYSEX_GetDeviceBoolean = '04';
export const SYSEX_SetDeviceConstructFull = '08'; // With Distributors
export const SYSEX_SetDeviceConstructOnly = '09'; // Without Distributors
export const SYSEX_SetDeviceName = '0A';
export const SYSEX_SetDeviceBoolean = '0B';

export const SYSEX_SetDistributor = '10';
export const SYSEX_RemoveDistributorID = '11';
export const SYSEX_RemoveAllDistributors = '12';
export const SYSEX_GetNumDistributors = '13';
export const SYSEX_GetAllDistributors = '14';
export const SYSEX_GetDistributorID = '15';
// export const SYSEX_SetDistributorID = '16';
export const SYSEX_SetDistributorID_Channels = '17';
export const SYSEX_SetDistributorID_Instruments = '18';
export const SYSEX_SetDistributorID_DistributionMethod = '19';
export const SYSEX_SetDistributorID_Boolean = '1A';
export const SYSEX_SetDistributorID_MinMaxNotes = '1B';
export const SYSEX_SetDistributorID_NumPolyphonicNotes = '1C';
