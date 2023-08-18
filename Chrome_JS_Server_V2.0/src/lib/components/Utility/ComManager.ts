import type { Device } from "../Devices/Device";
import { deviceListStore} from "$lib/store/stores";
import * as CONST from "./Constants";
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

        sysExCmd(device,CONST.SYSEX_SetDeviceName,asciiName);
        sysExCmd(device,CONST.SYSEX_SetDeviceBoolean,deviceBoolean);
        this.syncDevice(device);
    }

    public async saveDistributor(device: Device, distributor: Distributor){
        sysExCmd(device,CONST.SYSEX_SetDistributor,distributor.getConstruct());
        await this.syncDevice(device);
    }

    public async syncDevice(device: Device) {
        sysExCmd(device,CONST.SYSEX_GetDeviceConstructOnly,'');
        let deviceConstruct = await device.getConnection().receive();
        if(deviceConstruct === null) return;
        device.update(deviceConstruct);
        await this.syncDistributors(device);
    }

    public async syncDistributors(device: Device) {
        sysExCmd(device,CONST.SYSEX_GetNumDistributors,'');
        let numDistributors = await device.getConnection().receive();
        for(let i = 0; i<numDistributors[0]; i++){
            sysExCmd(device,CONST.SYSEX_GetDistributorID,to14BitStr(i));
            let distributorConstruct = await device.getConnection().receive();
            device.updateDistributor(distributorConstruct);
        }
    }

    public async removeDistributor(device: Device, distributor: Distributor) {
        sysExCmd(device,CONST.SYSEX_RemoveDistributorID,distributor.getId().toHexString());
        await this.syncDevice(device);
    }

    public async clearDistributors(device: Device) {
        sysExCmd(device,CONST.SYSEX_RemoveAllDistributors,"");
        await this.syncDevice(device);
    }

}

    ////////////////////////////////////////////////
    // Helper Functions
    ////////////////////////////////////////////////

function sysExCmd(device: Device, cmd:string, payload?:any){
    if (payload == 'undefined') payload = '';
    device.getConnection().sendHexString(
        CONST.SYSEX_START + to14BitStr(device.id) + cmd + payload + CONST.SYSEX_END
    );
    console.log(CONST.SYSEX_START + to14BitStr(device.id) + cmd + payload + CONST.SYSEX_END)
}

function to14BitStr(num: number){
    let result = (num & 0b01111111).toHexString();
    let result2 = ((num >> 8) & 0b01111111).toHexString();
    return(result2 + result);
}