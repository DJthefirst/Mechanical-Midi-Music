import Device from "./Device.js";
import SerialCom from "./SerialCom.js";

var deviceList = [];

export default class DeviceList {

    constructor () {
        this.selectedDevice = null;
    }

    async addDevice(ComType) {
        let device = new Device(ComType);
        await device.com.open();
        deviceList.push(device);
        device.com.port.ondisconnect = () => {this.removeDevice(device)};
        device.div.onclick = () => {this.selectDevice(device)};
        document.getElementById('deviceSelectList').append(device.getDiv());
        this.selectedDevice = device;
    }

    removeDevice(device) {
        if(device == null) device = this.selectedDevice;
        if(device == null) return;
        deviceList = deviceList.filter( dev => dev.getId() != device.getId());
        document.getElementById('deviceDiv' + this.selectedDevice.getId()).remove();
        device.com.close();
        this.selectedDevice = null;
    }

    selectDevice(device) {
        if (this.selectedDevice !== null) {
            document.getElementById('deviceDiv' + this.selectedDevice.id).style.backgroundColor = "#5086ad";
        }
        document.getElementById('deviceDiv' + device.id).style.backgroundColor = "#ffffff";
        this.selectedDevice = device;
    }

    getSelectedDevice(){ return this.selectedDevice; }

    static getDeviceList(){ return deviceList; }
}