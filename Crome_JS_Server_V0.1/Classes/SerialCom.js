export default class SerialCom{
    constructor() {
        this.reader = null;
        this.writer = null;
        this.port = null;
    }

    async open() {
        // Prompt user to select any serial port.
        let baudRate = document.getElementById("BaudRateDropDown").value;
        this.port = await navigator.serial.requestPort();
        await this.port.open({ baudRate: baudRate })
        this.reader = this.port.readable.getReader();
        this.writer = this.port.writable.getWriter();
        console.log(this.port.getInfo())
    }

    async close() {
        this.reader.releaseLock();
        this.writer.releaseLock();
        await this.port.close();
        }
    
    async readHexByteArray(){
        let byteArray = [];

        while(true){
            const { value, done } = await this.reader.read();
            console.log(value);
            if (done) {
                break;
            }    
        }
        return;
    }
    
    SendHexString(hexString) {
    
        // Convert String to Hex Array
        var hexBytes = new Uint8Array(Math.ceil(hexString.length / 2));
        for (var i = 0; i < hexBytes.length; i++) hexBytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
        
        //SendMsg
        this.SendHexByteArray(hexBytes);

    }

    async SendHexByteArray(hexByteArray) {
        console.log("Send: " + hexByteArray);
        
        // Convert Hex Array to Buffer
        let byteBuffer = hexByteArray.buffer.slice(hexByteArray.byteOffset, hexByteArray.byteLength + hexByteArray.byteOffset);

        //Writer
        this.writer.ready
            .then(() => this.writer.write(byteBuffer))
            .catch((err) => console.log("Send error:",err));
    }
}