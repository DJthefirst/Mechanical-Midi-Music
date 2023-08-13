import { Device } from "../Devices/Device";
import SerialConnection from "./SerialConnection";
import { deviceListStore } from "$lib/store/stores";
import {} from "./Constants";
import { SYSEX_CMD_GetDeviceConstructOnly, SYSEX_CMD_GetDistributorID, SYSEX_CMD_GetNumDistributors, SYSEX_END, SYSEX_START } from "./Constants";
import {} from "./helpers"

let i = 1;
//let name = ['Dulcimer', 'AirCompressor', 'FloppyDrive','StepperMotor'];
//let platform = ['ESP32','Ardino Nano', 'Arduino Uno', 'Arduino Mega'];
//let deviceType = ['Shift Register', 'PWM Driver', 'FloppyDrive','StepperMotor'];

let deviceList: any;

deviceListStore.subscribe((prev_value: any) => deviceList = prev_value);

export interface Connection {
    open: (baudRate:number) => void;
    close: () => void;
    sendHexString: (msg: string) => void;
    sendHexByteArray: (msg: Uint8Array) => void;
    receive : () => any;
}

export default class SerialManager{

    public async addDevice(baudRate: number){
        let connection = new SerialConnection();
        await connection.open(baudRate);
        // Fix this to not require Dev ID on init
        let device = new Device(connection, 0, "temp", '000.0', "Platfourm", "device", 0, 0, 127);
        this.syncDevice(device);
        deviceList.push(device);
        deviceListStore.set(deviceList);
    }

    public sendMidiToDevices(msg: any) {
        for (let device of deviceList){
            device.getConnection().sendHexByteArray(msg);
        }
    }

    ////////////////////////////////////////////////
    // SYSEX Commands
    ////////////////////////////////////////////////

    public async syncDevice(device: Device) {
        sysExCmd(device,SYSEX_CMD_GetDeviceConstructOnly,'');
        let deviceConstruct = await device.getConnection().receive();
        device.update(deviceConstruct);
        this.syncDistributors(device);
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