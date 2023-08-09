import DeviceList__SvelteComponent_ from "../Devices/DeviceList.svelte";

export default class SerialConnection{
    private reader: any;
    private writer: any;
    private port: any;

    constructor() {
        this.reader = null;
        this.writer = null;
        this.port = null;
    }

    async open(baudRate: number) {
        // @ts-ignore 
        this.port = await navigator.serial.requestPort(); // Prompt user to select any serial port.
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
    
    //readHexByteArray
    async receive(){
        let byteArray:any = [];

        while(true){
            const { value, done } = await this.reader.read();
            console.log(value);
            if (done) {
                break;
            }    
        }
        return byteArray;
    }

    async sendHexString(hexString: string) {
    
        // Convert String to Hex Array
        let hexBytes = new Uint8Array(Math.ceil(hexString.length / 2));
        for (let i = 0; i < hexBytes.length; i++) hexBytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
        
        //SendMsg
        await this.sendHexByteArray(hexBytes);

    }

    async sendHexByteArray(hexByteArray: any) {
        console.log("Send: " + hexByteArray);
        hexByteArray = new Uint8ClampedArray(hexByteArray);

        // Convert Hex Array to Buffer
        let byteBuffer = hexByteArray.buffer.slice(hexByteArray.byteOffset, hexByteArray.byteLength + hexByteArray.byteOffset);

        //Writer
        this.writer.ready
            .then(() => this.writer.write(byteBuffer))
            .catch((err: any) => console.log("Send error:",err));
    }
}