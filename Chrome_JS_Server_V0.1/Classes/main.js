import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import MidiController from "./MidiControler.js";

let deviceList = new DeviceList();
let midiController = new MidiController();

//document.getElementById("btnStart").onclick = () => { main() };
document.getElementById("btnGetMidiPorts").onclick = () => {midiController.updateMidiSources()}

document.getElementById("btnAddDevice").onclick = async () => { deviceList.addDevice('SerialCOM X')};
document.getElementById("btnRemoveDevice").onclick = () => { deviceList.removeDevice() };

document.getElementById("btnSendHex").onclick = () => { deviceList.selectedDevice.com.SendHexString("") };
document.getElementById("btnNoteOn").onclick = () => { deviceList.selectedDevice.com.SendHexString('903939') };
document.getElementById("btnNoteOff").onclick = () => { deviceList.selectedDevice.com.SendHexString('803939') };
document.getElementById("btnGetConfig").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D00010100F7') };
document.getElementById("btnSetRobin").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D000117000002F7') };
document.getElementById("btnSetStraight").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D000117000003F7') };

main();

async function main() {
    const intervalId = setInterval(() => {
        for (let device of DeviceList.getDeviceList()) {
            device.com.readHexByteArray()
        }
    }, 500);
}