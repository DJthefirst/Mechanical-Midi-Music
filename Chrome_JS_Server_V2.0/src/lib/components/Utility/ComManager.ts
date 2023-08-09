import { Device } from "../Devices/Device";
import SerialConnection from "./SerialConnection";
import { deviceListStore } from "$lib/store/stores";

let i = 1;
let name = ['Dulcimer', 'AirCompressor', 'FloppyDrive','StepperMotor'];
let platform = ['ESP32','Ardino Nano', 'Arduino Uno', 'Arduino Mega'];
let deviceType = ['Shift Register', 'PWM Driver', 'FloppyDrive','StepperMotor'];

let deviceList: any;

deviceListStore.subscribe((prev_value) => deviceList = prev_value);


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
        let device = new Device(connection, i, name[i], '001.1', platform[i], deviceType[i], i*4, 0, 127);
        this.syncDistributors(device);
        deviceList.push(device);
        deviceListStore.set(deviceList);
    }

    public sendMidiToDevices(msg: any) {
        for (let device of deviceList){
            device.getConnection().sendHexByteArray(msg);
        }
    }


    public async syncDevice(device: Device) {
        sysExCmd(device,SYSEX_CMD_GetDistributorConstructID,'0000');
    }

    public async syncDistributors(device: Device) {
        sysExCmd(device,SYSEX_CMD_GetNumDistributors,'');
        //sysExCmd(device,SYSEX_CMD_GetDistributorConstructID,'0000');
        //sysExCmd(device,SYSEX_CMD_SetDistributionMethod,'000003');
        console.log("waiting for result");
        let result = await device.getConnection().receive();
        console.log("result: " + String(result))
    }
}

function sysExCmd(device: Device, cmd:string, payload?:any){
    if (payload == 'undefined') payload = '';
    device.getConnection().sendHexString(
    SYSEX_START + to14Bit(device.id) + cmd + payload + SYSEX_END
    );
    console.log(SYSEX_START + to14Bit(device.id) + cmd + payload + SYSEX_END)
}

function to14Bit(num: number){
    let result = (num & 0b01111111).toString(16);
    result = (result.length < 2) ? '0' + result : result;
    let result2 = ((num >> 8) & 0b01111111).toString(16);
    result2 = (result2.length < 2) ? '0' + result2 : result2;
    return(result2 + result);
}

const SYSEX_START = 'F07D';
const SYSEX_END = 'F7';

const SYSEX_CMD_GetDeviceConstruct = '02';
const SYSEX_CMD_GetNumDistributors = '13';
const SYSEX_CMD_GetDistributorConstructID = '15';
const SYSEX_CMD_SetDistributionMethod = '17';