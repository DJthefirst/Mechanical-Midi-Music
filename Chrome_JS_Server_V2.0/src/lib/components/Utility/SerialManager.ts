import { Device } from "../Devices/Device";
import SerialConnection from "./SerialConnection";
import { deviceListStore } from "$lib/store/stores";
import { onDestroy } from "svelte";

let i = 1;
let name = ['Dulcimer', 'AirCompressor', 'FloppyDrive','StepperMotor'];
let platform = ['ESP32','Ardino Nano', 'Arduino Uno', 'Arduino Mega'];
let deviceType = ['Shift Register', 'PWM Driver', 'FloppyDrive','StepperMotor'];

let deviceList:Device[] = [];

deviceListStore.subscribe((prev_value) => deviceList = prev_value);


export interface Connection {
    open: (baudRate:number) => void;
    close: () => void;
    sendHexString: (msg: string) => void;
    sendHexByteArray: (msg: Uint8Array) => void;
    receive: () => any;
}

export default class SerialManager{

    public async addDevice(baudRate: number){
        let connection = new SerialConnection();
        await connection.open(baudRate);
        let device = new Device(connection, i, name[i], '001.1', platform[i], deviceType[i], i*4, 0, 127);
        deviceList.push(device);
        deviceListStore.set(deviceList);
    }

    public sendMidiToDevices(msg: any) {
        for (let device of deviceList){
            device.getConnection().sendHexByteArray(msg);
        }
    }
}