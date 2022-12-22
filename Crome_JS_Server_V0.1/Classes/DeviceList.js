import Device from "./Device.js";
import Network from "./Network.js";

export default class DeviceList {

    constructor () {
        this.selectedDevice = null;
        this.deviceList = [];
    }

    addDevice(port, ComType) {
        let device = new Device(port, ComType);
        this.deviceList.push(device);
        port.ondisconnect = () => {this.removeDevice(device)};
        device.div.onclick = () => {this.selectDevice(device)};
        document.getElementById('deviceSelectList').append(device.getDiv());
        this.selectedDevice = device;
    }

    removeDevice(device) {
        if(device == null) device = this.selectedDevice;
        if(device == null) return;
        this.deviceList = this.deviceList.filter( dev => dev.getId() != device.getId());
        document.getElementById('deviceDiv' + this.selectedDevice.getId()).remove();
        Network.closePort(device.getPort());
        this.selectedDevice = null;
    }

    selectDevice(device) {
        if (this.selectedDevice !== null) {
            document.getElementById('deviceDiv' + this.selectedDevice.id).style.backgroundColor = "#5086ad";
        }
        document.getElementById('deviceDiv' + device.id).style.backgroundColor = "#ffffff";
        this.selectedDevice = device;
    }
}