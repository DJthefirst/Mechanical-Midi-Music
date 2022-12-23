import Device from "./Device.js";
import Network from "./Network.js";

    var selectedDevice = null;
    var deviceList = [];

export default class DeviceList {

    constructor () {}

    addDevice(port, ComType) {
        let device = new Device(port, ComType);
        deviceList.push(device);
        port.ondisconnect = () => {this.removeDevice(device)};
        device.div.onclick = () => {this.selectDevice(device)};
        document.getElementById('deviceSelectList').append(device.getDiv());
        selectedDevice = device;
    }

    removeDevice(device) {
        if(device == null) device = selectedDevice;
        if(device == null) return;
        deviceList = deviceList.filter( dev => dev.getId() != device.getId());
        document.getElementById('deviceDiv' + selectedDevice.getId()).remove();
        Network.closePort(device.getPort());
        selectedDevice = null;
    }

    selectDevice(device) {
        if (selectedDevice !== null) {
            document.getElementById('deviceDiv' + selectedDevice.id).style.backgroundColor = "#5086ad";
        }
        document.getElementById('deviceDiv' + device.id).style.backgroundColor = "#ffffff";
        selectedDevice = device;
    }

    static getSelectedDevice(){ return selectedDevice; }

    static getDeviceList(){ return deviceList; }
}