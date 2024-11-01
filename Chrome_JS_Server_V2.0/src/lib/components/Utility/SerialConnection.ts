import * as CONST from './Constants';
import { SYSEX_RegExpEnd, SYSEX_RegExpMsg } from './Constants';

export default class SerialConnection {
	private reader: any;
	private writer: any;
	private port: any;

	constructor() {
		this.reader = null;
		this.writer = null;
		this.port = null;
	}

	getPort() {
		return this.port;
	}

	async open(baudRate: number) {
		// @ts-ignore
		this.port = await navigator.serial.requestPort(); // Prompt user to select any serial port.
		await this.port.open({ baudRate: baudRate });
		this.reader = this.port.readable.getReader();
		this.writer = this.port.writable.getWriter();

		// let stall = true;
		// setTimeout(()=> {stall = false},5000);
		
		// while(stall){
		// 	let msg = await this.receive(500);
		// 	if (msg != null && msg[0] == Number(CONST.SYSEX_DeviceReady)) break;
		// }

		//Request if device ready If device is not ready there will be no response
		this.sendHexString(CONST.SYSEX_START + CONST.SYSEX_ServerID + CONST.SYSEX_BroadcastID + CONST.SYSEX_DeviceReady + CONST.SYSEX_END);

		//Add Timeout and Delay for first connection.
		let stall2 = true;
		setTimeout(()=> {stall2 = false},5000);
		
		while(stall2){
			let msg = await this.receive(500);
			if (msg != null && msg[0] == Number(CONST.SYSEX_DeviceReady)) return;
		}

	}

	async close() {
		this.reader.releaseLock();
		this.writer.releaseLock();
		await this.port.close();
	}

	//readHexByteArray
	async receive(timeoutLen = 0): Promise<Uint8Array | null> {
		let hexString: string = '';
		let result;

		let stall = true;
		let timeOutID;
		if(timeoutLen != 0) timeOutID = setTimeout(()=> {stall = false},timeoutLen);

		while(stall){
			//@ts-ignore
			const {value, done} = await this.reader.read();
			for (let num of value) {
				hexString += num.toHexString();
				clearTimeout(timeOutID);
				timeOutID = setTimeout(()=> {stall = false}, 500);
				stall = true;
			}
			
				
			if ((result = SYSEX_RegExpMsg.exec(hexString)) !== null) {
				return result[1].toHex();
			}

			if ((result = SYSEX_RegExpEnd.exec(hexString)) !== null) {
				console.log('Bad Msg' + String(result[0]));
				return null;
				
			}
		}
		console.log('Msg Timedout');
		return null;
	}




	async sendHexString(hexString: string) {
		//SendMsg
		await this.sendHexByteArray(hexString.toHex());
	}

	async sendHexByteArray(hexByteArray: any) {
		// console.log("Send: " + hexByteArray);
		hexByteArray = new Uint8ClampedArray(hexByteArray);
		// Convert Hex Array to Buffer
		let byteBuffer = hexByteArray.buffer.slice(
			hexByteArray.byteOffset,
			hexByteArray.byteLength + hexByteArray.byteOffset
		);

		//Writer
		this.writer.ready
			.then(() => this.writer.write(byteBuffer))
			.catch((err: any) => console.log('Send error:', err));
	}
}

function sleep(ms: number) {
	return new Promise((resolve) => setTimeout(resolve, ms || 1000));
}
