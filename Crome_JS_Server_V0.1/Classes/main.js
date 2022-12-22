import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import Network from "./Network.js";

let deviceList = new DeviceList();
let network = new Network();

document.getElementById("btnStart").onclick = () => { main() };

document.getElementById("btnAddDevice").onclick = async () => { deviceList.addDevice(await Network.openPort(), 'SerialCOM X')};
document.getElementById("btnRemoveDevice").onclick = () => { deviceList.removeDevice() };

document.getElementById("btnSendHex").onclick = () => { Network.SendHexString(deviceList.selectedDevice.getPort(),"") };
document.getElementById("btnNoteOn").onclick = () => { Network.SendHexString(deviceList.selectedDevice.getPort(),'903939') };
document.getElementById("btnNoteOff").onclick = () => { Network.SendHexString(deviceList.selectedDevice.getPort(),'803939') };
document.getElementById("btnGetConfig").onclick = () => { Network.SendHexString(deviceList.selectedDevice.getPort(),'F07D00000100F7') };

