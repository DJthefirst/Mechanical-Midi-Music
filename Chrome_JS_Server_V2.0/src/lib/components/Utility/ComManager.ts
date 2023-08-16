import type { Device } from "../Devices/Device";
import { deviceListStore} from "$lib/store/stores";
import { SYSEX_CMD_SetDeviceBoolean, SYSEX_CMD_SetDeviceName, SYSEX_CMD_SetDistributor} from "./Constants";
import { SYSEX_CMD_GetDeviceConstructOnly, SYSEX_CMD_GetDistributorID, SYSEX_CMD_GetNumDistributors, SYSEX_END, SYSEX_START } from "./Constants";

import {} from "./helpers"
import type { Distributor } from "../Distributors/Distributor";
import SerialConnection from "./SerialConnection";

let i = 1;
let deviceList: Device[];

deviceListStore.subscribe((prev_value: any) => deviceList = prev_value);

export interface Connection {
    open: (baudRate:number) => void;
    close: () => void;
    sendHexString: (msg: string) => void;
    sendHexByteArray: (msg: Uint8Array) => void;
    receive : () => any;
    getPort: () => any;
}

// Hanlde Serial Communications
export default class SerialManager{

    public sendMidiToDevices(msg: any) {
        for (let device of deviceList){
            device.getConnection().sendHexByteArray(msg);
        }
    }

    ////////////////////////////////////////////////
    // SYSEX Commands
    ////////////////////////////////////////////////

    public async saveDevice(device: Device){
        let asciiName = device.name.toAsciiString();
        while (asciiName.length < 40 ) asciiName += "00";
        let deviceBoolean = device.isOnmiMode ? 0x01 : 0x00;

        sysExCmd(device,SYSEX_CMD_SetDeviceName,asciiName);
        sysExCmd(device,SYSEX_CMD_SetDeviceBoolean,deviceBoolean);
        this.syncDevice(device);
    }

    public async saveDistributor(device: Device, distributor: Distributor){
        sysExCmd(device,SYSEX_CMD_SetDistributor,distributor.getConstruct());
        await this.syncDevice(device);
    }

    public async syncDevice(device: Device) {
        sysExCmd(device,SYSEX_CMD_GetDeviceConstructOnly,'');
        let deviceConstruct = await device.getConnection().receive();
        if(deviceConstruct === null) return;
        device.update(deviceConstruct);
        await this.syncDistributors(device);
    }

    public async syncDistributors(device: Device) {
        sysExCmd(device,SYSEX_CMD_GetNumDistributors,'');
        let numDistributors = await device.getConnection().receive();
        for(let i = 0; i<numDistributors[0]; i++){
            sysExCmd(device,SYSEX_CMD_GetDistributorID,to14BitStr(i));
            let distributorConstruct = await device.getConnection().receive();
            device.updateDistributor(distributorConstruct);
        }
    }
}

    ////////////////////////////////////////////////
    // Helper Functions
    ////////////////////////////////////////////////

function sysExCmd(device: Device, cmd:string, payload?:any){
    if (payload == 'undefined') payload = '';
    device.getConnection().sendHexString(
    SYSEX_START + to14BitStr(device.id) + cmd + payload + SYSEX_END
    );
    console.log(SYSEX_START + to14BitStr(device.id) + cmd + payload + SYSEX_END)
}

function to14BitStr(num: number){
    let result = (num & 0b01111111).toHexString();
    let result2 = ((num >> 8) & 0b01111111).toHexString();
    return(result2 + result);
}