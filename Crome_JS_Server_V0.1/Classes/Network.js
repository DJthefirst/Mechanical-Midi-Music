export default class Network{

    static async openPort() {
        // Prompt user to select any serial port.
        const port = await navigator.serial.requestPort();
        await port.open({ baudRate: 115200 });
        console.log(port.getInfo())
        return port;
    }

    static async closePort(port) {
        await port.close();
    }
    
    SendHexString(port,hexString) {
    
        // Convert String to Hex Array
        var hexBytes = new Uint8Array(Math.ceil(hexString.length / 2));
        for (var i = 0; i < hexBytes.length; i++) hexBytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
        console.log(hexBytes);
        
        //SendMsg
        this.SendHexByteArray(hexBytes);
    }

    async SendHexByteArray(port,hexByteArray) {
    
        // Convert Hex Array to Buffer
        let byteBuffer = hexByteArray.buffer.slice(hexByteArray.byteOffset, hexByteArray.byteLength + hexByteArray.byteOffset);
    
        let writer = port.writable.getWriter();
        writer.ready
            .then(() => writer.write(byteBuffer))
            .then(() => console.log("Sent Byte."))
            .catch((err) => console.log("Send error:",err));
    
        writer.ready
            .then(() => writer.close())
            .then(() => console.log("Writer Colsed."))
            .catch((err) => console.log("Send error:",err));
    }
}