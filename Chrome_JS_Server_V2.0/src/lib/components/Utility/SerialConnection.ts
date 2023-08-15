import { SYSEX_RegExpEnd, SYSEX_RegExpMsg } from "./Constants";

export default class SerialConnection{
    private reader: any;
    private writer: any;
    private port: any;

    constructor() {
        this.reader = null;
        this.writer = null;
        this.port = null;
    }

    getPort(){
        return this.port;
    }

    async open(baudRate: number) {
        // @ts-ignore 
        this.port = await navigator.serial.requestPort(); // Prompt user to select any serial port.
        await this.port.open({ baudRate: baudRate });
        await sleep(200); // ESP32 D15->GND to Suppress Boot Messages, Delay for Boot
        this.reader = this.port.readable.getReader();
        this.writer = this.port.writable.getWriter();
    }

    async close() {
        this.reader.releaseLock();
        this.writer.releaseLock();
        await this.port.close();
        }
    
    //readHexByteArray
    async receive(): Promise<Uint8Array | null>{
        let hexString: string = "";
        let result;
        while(true){
            if ((result = SYSEX_RegExpMsg.exec(hexString)) !== null){ 
                return result[1].toHex();
            }
            if ((result = SYSEX_RegExpEnd.exec(hexString)) !== null){
                console.log('Bad Msg' + String(result[0]));
                return null;
            }

            const { value, done } = await this.reader.read();
            for(let num of value){ 
                hexString += num.toHexString();
            }
        }
    }

    async sendHexString(hexString: string) {
        //SendMsg
        await this.sendHexByteArray(hexString.toHex());

    }

    async sendHexByteArray(hexByteArray: any) {
        // console.log("Send: " + hexByteArray);
        hexByteArray = new Uint8ClampedArray(hexByteArray);
        // Convert Hex Array to Buffer
        let byteBuffer = hexByteArray.buffer.slice(hexByteArray.byteOffset, hexByteArray.byteLength + hexByteArray.byteOffset);

        //Writer
        this.writer.ready
            .then(() => this.writer.write(byteBuffer))
            .catch((err: any) => console.log("Send error:",err));
    }
}

function sleep(ms: number) {
    return new Promise(resolve => setTimeout(resolve, ms || 1000));
  }