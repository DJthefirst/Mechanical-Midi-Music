import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import Network from "./Network.js";
import MidiController from "./MidiControler.js";

let deviceList = new DeviceList();
let network = new Network();
let midiController = new MidiController();


function selectedPort() {deviceList.selectedDevice.getPort();}

document.getElementById("btnStart").onclick = () => { main() };
document.getElementById("btnGetMidiPorts").onclick = () => {midiController.updateMidiSources()}

document.getElementById("btnAddDevice").onclick = async () => { deviceList.addDevice(await Network.openPort(), 'SerialCOM X')};
document.getElementById("btnRemoveDevice").onclick = () => { deviceList.removeDevice() };

document.getElementById("btnSendHex").onclick = () => { Network.SendHexString(selectedPort(),"") };
document.getElementById("btnNoteOn").onclick = () => { Network.SendHexString(selectedPort(),'903939') };
document.getElementById("btnNoteOff").onclick = () => { Network.SendHexString(selectedPort(),'803939') };
document.getElementById("btnGetConfig").onclick = () => { Network.SendHexString(selectedPort(),'F07D00000100F7') };

function main() {

}