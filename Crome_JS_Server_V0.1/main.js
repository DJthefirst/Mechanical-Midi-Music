import {DevicePool, Device} from "./classes.js"

if ("serial" in navigator) console.log("The Web Serial API is supported.");

document.getElementById("btnStart").onclick = () => { main() };
document.getElementById("btnAddDevice").onclick = () => { openPort() };
document.getElementById("btnRemoveDevice").onclick = () => { removeDevice() };
document.getElementById("btnSendHex").onclick = () => { SendHexText() };
document.getElementById("btnNoteOn").onclick = () => { SendHex('903939') };
document.getElementById("btnNoteOff").onclick = () => { SendHex('803939') };
document.getElementById("btnGetConfig").onclick = () => { SendHex('F07D00000100F7') };


var devicePool = new DevicePool;

function removeDevice() {
    devicePool.getSelectedDevice().getPort().close();
    devicePool.remove();
}

async function openPort() {
    // Prompt user to select any serial port.
    const port = await navigator.serial.requestPort();

    await port.open({ baudRate: 115200 });

    //const reader = port.readable.getReader();
    //const writer = port.writable.getWriter();
    devicePool.add( new Device(port, 'SerialCOM X'))
}

async function SendHexText() {
    var hexString = document.getElementById('textSendHex').value;
    SendHex(hexString)
}

async function SendHex(hexString) {
    //Guard for no selected device
    if(devicePool.getSelectedDevice() == null) return;

    // Convert String to Hex Array
    var hexBytes = new Uint8Array(Math.ceil(hexString.length / 2));
    for (var i = 0; i < hexBytes.length; i++) hexBytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
    console.log(hexBytes);
    let byteBuffer = hexBytes.buffer.slice(hexBytes.byteOffset, hexBytes.byteLength + hexBytes.byteOffset);

    let writer = devicePool.getSelectedDevice().getPort().writable.getWriter();
    writer.ready
        .then(() => writer.write(byteBuffer))
        .then(() => console.log("Sent Byte."))
        .catch((err) => console.log("Send error:",err));

    writer.ready
        .then(() => writer.close())
        .then(() => console.log("Writer Colsed."))
        .catch((err) => console.log("Send error:",err));
}

async function main() {
    // Listen to data coming from the serial device.
    while (true) {
        let devices = devicePool.array
        for (var i = 0; i < devices.length; i++) {
            let reader = devices[i].getPort().readable.getReader();
            const { value, done } = await reader.read();
            if (done) {
                // Allow the serial port to be closed later.
                reader.releaseLock();
                break;
            }
            // value is a Uint8Array.
            document.getElementById("my_label").innerHTML = value;
            console.log(value);
        }
    }
}