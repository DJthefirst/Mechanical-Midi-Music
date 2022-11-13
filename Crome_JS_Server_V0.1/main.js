//const { disconnect } = require("process");
//const { writer } = require("repl");

if ("serial" in navigator) console.log("The Web Serial API is supported.");

document.getElementById("btnStart").onclick = () => { main() };
document.getElementById("btnAddDevice").onclick = () => { openPort() };
document.getElementById("btnRemoveDevice").onclick = () => { removeDevice() };
document.getElementById("btnSendHex").onclick = () => { SendHexText() };
document.getElementById("btnNoteOn").onclick = () => { SendHex('903939') };
document.getElementById("btnNoteOff").onclick = () => { SendHex('803939') };
document.getElementById("btnGetConfig").onclick = () => { SendHex('F07D00000100F7') };

var currentDevId = 0;
var selectedDevice = null;

class Device {

    constructor(reader, writer, ComType) {
      this.reader = reader;
      this.writer = writer;
      this.comType = ComType;
      this.instrumentType = 'Instrument';
      this.id = currentDevId;
      currentDevId++;

      /*this.config = null; */
      this.getConfig();

      this.createDiv();

    }

    getReader () {
        return this.reader;
    }

    getWriter () {
        return this.writer;
    }

    getId () {
        return this.id;
    }

    selectDevice () {
        console.log('Selected Device: ' + this.id);
        selectedDevice = this;
        Array.from(document.getElementsByClassName('deviceDiv'))
            .forEach(function(element) {element.style.backgroundColor = "#5086ad"});
        document.getElementById('deviceDiv' + this.id).style.backgroundColor = 'white';
    }

    createDiv () {
        let div = document.createElement('div');
        div.id = 'deviceDiv' + this.id;
        
        div.className = 'deviceDiv';
        div.appendChild(elementFromHTML(' \
            <div class="deviceInfo"> \
                <b class="deviceInfo">Dev: '+ this.id +'</b>\
                <b class="deviceInfo"> ' + this.instrumentType + '</b>\
                <b class="deviceInfo">' + this.comType +'</b>\
            </div> \
            <div> \
            </div> \
        '));

        div.onclick = () => {this.selectDevice()};
        document.getElementById('windowRightBar').append(div);
    }

    getConfig () {

    }
}

class DevicePool{
    constructor (){
        this.devices = new Array();
    }

    add (device) {
        this.devices.push(device);
    }

    remove () {
        this.devices = this.devices.filter(function(device, index, arr){
            return (device.getId() != selectedDevice.getId());
        });
        document.getElementById('deviceDiv' + selectedDevice.id).remove();
        selectedDevice = null;
    }

    get array () {
        return this.devices;
    }
}

var devicePool = new DevicePool;

function elementFromHTML (html) {
    const template = document.createElement("template");
    template.innerHTML = html.trim();
    return template.content.firstElementChild;
}

function removeDevice() {
    devicePool.remove();
}

async function openPort() {
    // Prompt user to select any serial port.
    const port = await navigator.serial.requestPort();

    await port.open({ baudRate: 115200 });

    const reader = port.readable.getReader();
    const writer = port.writable.getWriter();
    devicePool.add( new Device(reader, writer, 'SerialCOM X'))
}

async function SendHexText() {
    var hexString = document.getElementById('textSendHex').value;
    SendHex(hexString)
}

async function SendHex(hexString) {
    //Guard for no selected device
    if(selectedDevice == null) return;

    // Convert String to Hex Array
    var hexBytes = new Uint8Array(Math.ceil(hexString.length / 2));
    for (var i = 0; i < hexBytes.length; i++) hexBytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
    console.log(hexBytes);
    byteBuffer = hexBytes.buffer.slice(hexBytes.byteOffset, hexBytes.byteLength + hexBytes.byteOffset);

    let writer = selectedDevice.getWriter();
    writer.ready
        .then(() => writer.write(byteBuffer))
        .then(() => console.log("Sent Byte."))
        .catch((err) => console.log("Send error:",err));



    //writer.ready
    //    .then(() => writer.close())
    //    .then(() => console.log("Message Sent."))
    //    .catch((err) => console.log("Send error:",err));
}

async function main() {
    // Listen to data coming from the serial device.
    while (true) {
        let devices = devicePool.array
        for (var i = 0; i < devices.length; i++) {
            let reader = devices[i].getReader();
            const { value, done } = await reader.read();
            if (done) {
                // Allow the serial port to be closed later.
                reader.releaseLock();
                break;
            }
            // value is a Uint8Array.
            document.getElementById("my_label").innerHTML += value;
            console.log(value);
        }
    }
}