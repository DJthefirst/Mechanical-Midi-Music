if ("serial" in navigator) console.log("The Web Serial API is supported.");

document.getElementById("btnStart").onclick = () => { main() };
document.getElementById("btnAddDevice").onclick = () => { openPort() };
document.getElementById("btnSendHex").onclick = () => { SendHex() };


class Device {
    constructor(reader, writer) {
      this.reader = reader;
      this.writer = writer;
    }

    getReader () {
        return this.reader;
    }

    getWriter () {
        return this.writer;
    }
}

class DevicePool{
    constructor (){
        this.devices = new Array();
    }

    add (device) {
        this.devices.push(device);
    }

    get array () {
        return this.devices;
    }
}

let devicePool = new DevicePool;

async function openPort() {
    console.log("click");
    // Prompt user to select any serial port.
    const port = await navigator.serial.requestPort();

    await port.open({ baudRate: 115200 });

    const reader = port.readable.getReader();
    const writer = port.writable.getWriter();
    devicePool.add( new Device(reader, writer))
}

async function SendHex() {
    var hex = parseInt(document.getElementById('textSendHex').value);
    let buffer = new Uint8Array(1);
    buffer[0] = hex;
    writer = devicePool.array[0].getWriter();
    await writer.write(buffer);
    writer.releaseLock();
    console.log(buffer);
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