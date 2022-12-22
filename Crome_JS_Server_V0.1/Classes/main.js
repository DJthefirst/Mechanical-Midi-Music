import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import Network from "./Network.js";

let deviceList = new DeviceList();
let network = new Network();

document.getElementById("btnStart").onclick = () => { main() };

document.getElementById("btnAddDevice").onclick = async () => { deviceList.addDevice(await Network.openPort(), 'SerialCOM X')};
document.getElementById("btnRemoveDevice").onclick = () => { deviceList.removeDevice() };

document.getElementById("btnSendHex").onclick = () => { SendHexString() };
document.getElementById("btnNoteOn").onclick = () => { SendHexString('903939') };
document.getElementById("btnNoteOff").onclick = () => { SendHexString('803939') };
document.getElementById("btnGetConfig").onclick = () => { SendHexString('F07D00000100F7') };

