import DeviceList__SvelteComponent_ from "../Devices/DeviceList.svelte";
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
    async receive(): Promise<Uint8Array>{
        let hexString: string = "";
        let result;
        while(true){
            if ((result = SYSEX_RegExpMsg.exec(hexString)) !== null){ //@ts-ignore
                hexString = hexString.slice(result.indices[0][1]);
                return result[1].toHex();
            }
            if ((result = SYSEX_RegExpEnd.exec(hexString)) !== null){ //@ts-ignore
                hexString = hexString.slice(result.indices[0][1]);
                console.log('Bad Msg' + String(result[0]));
                continue;
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