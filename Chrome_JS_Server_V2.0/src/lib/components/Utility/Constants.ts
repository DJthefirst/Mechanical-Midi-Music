export const SYSEX_RegExpMsg = /F07D(?:[0-7][0-9a-fA-F]){2}7F7F((?:[0-7][0-9a-fA-F])*)F7/di;
export const SYSEX_RegExpEnd = /^(?:[0-9a-fA-F][0-9a-fA-F])*F7/di;

export const SYSEX_START = 'F07D';
export const SYSEX_END = 'F7';
export const SYSEX_BroadcastID = '0000';
export const SYSEX_ServerID = '7F7F';

export const SYSEX_DeviceReady = '00';
export const SYSEX_ResetDeviceCfg = '01';
export const SYSEX_DiscoverDevices = '02';
export const SYSEX_Message = '03';

export const SYSEX_GetDeviceConstructFull = '10'; // With Distributors
export const SYSEX_GetDeviceConstructOnly = '11'; // Without Distributors
export const SYSEX_GetDeviceName = '12';
export const SYSEX_GetDeviceBoolean = '13';

export const SYSEX_SetDeviceConstructFull = '20'; // With Distributors
export const SYSEX_SetDeviceConstructOnly = '21'; // Without Distributors
export const SYSEX_SetDeviceName = '22';
export const SYSEX_SetDeviceBoolean = '23';

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

