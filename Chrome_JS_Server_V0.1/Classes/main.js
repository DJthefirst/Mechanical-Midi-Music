import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import MidiController from "./MidiControler.js";

let deviceList = new DeviceList();
let midiController = new MidiController();

//document.getElementById("btnGetMidiPorts").onclick = () => {midiController.updateMidiSources()}

document.getElementById("btnAddDevice").onclick = async () => { deviceList.addDevice('SerialCOM X')};
document.getElementById("btnRemoveDevice").onclick = () => { deviceList.removeDevice() };

document.getElementById("btnSendHex").onclick = () => { deviceList.selectedDevice.com.SendHexString(document.getElementById("textSendHex").value)};
document.getElementById("btnNoteOn").onclick = () => { deviceList.selectedDevice.com.SendHexString('903939') };
document.getElementById("btnNoteOff").onclick = () => { deviceList.selectedDevice.com.SendHexString('803939') };
document.getElementById("btnStopAllNotes").onclick = () => { deviceList.selectedDevice.com.SendHexString('B07B00') };
document.getElementById("btnGetConfig").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D00010100F7') };
document.getElementById("btnSetRobin").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D000117000002F7') };
document.getElementById("btnSetStraight").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D000117000003F7') };

//Add Distributor
document.getElementById("btnAddDistributor").onclick = () => { 

    //Compess binary flags into Bytes
    let binaryValues = 0x00;
    if (document.getElementById("DamperEn").checked == true) binaryValues |= 0x01;
    if (document.getElementById("PolyphonicEn").checked == true) binaryValues |= 0x02;
    if (document.getElementById("NoteOverwrite").checked == true) binaryValues |= 0x04;

    deviceList.selectedDevice.com.SendHexString(
    'F07D000110' + '0000' + 
    Hex8to7bit(document.getElementById("Channels").value,3) +
    Hex8to7bit(document.getElementById("Instruments").value,5) +
    Hex8to7bit(document.getElementById("DistributionMethod").value,1) +
    Hex8to7bit(binaryValues.toString(16),1) +
    Hex8to7bit(document.getElementById("NoteMin").value,1) +
    Hex8to7bit(document.getElementById("NoteMax").value,1) +
    Hex8to7bit(document.getElementById("Polyphonic").value,1) +
    '00' + //Reserved
    'F7');
};
document.getElementById("btnRemoveDistributor").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D00011100F7') }; //Removes Distributor 0
document.getElementById("btnRemoveAllDistributors").onclick = () => { deviceList.selectedDevice.com.SendHexString('F07D000112F7') };

main();

async function main() {
    const intervalId = setInterval(() => {
        for (let device of DeviceList.getDeviceList()) {
            device.com.readHexByteArray()
        }
    }, 500);
}

//Converts a 8bit hex string to 7Bit hex string
function Hex8to7bit(input,len){
    let hex = parseInt(input,16);
    let output = "";
    for(let i = 0; i < len; i++){ 
        let result = (hex & 0x7F).toString(16);
        if (result.length % 2 != 0) result = 0 + result;
        output += result;
        hex = hex >> 7; 
    }
    return(output);
  }