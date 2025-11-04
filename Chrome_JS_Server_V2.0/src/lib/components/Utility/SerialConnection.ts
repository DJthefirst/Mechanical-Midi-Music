import * as CONST from './Constants';
import { MMM_Msg } from './MMM_Msg';
import { MessageHandler } from './MessageHandler';
import type { Device } from '../Devices/Device';

/**
 * SerialConnection - Manages serial port communication
 * Opens port, sends ready broadcast, and routes all messages through MessageHandler
 */
export default class SerialConnection {
	private reader: ReadableStreamDefaultReader<Uint8Array> | null;
	private writer: WritableStreamDefaultWriter<Uint8Array> | null;
	private port: any;
	private messageHandler: MessageHandler | null;
	private isRunning: boolean;
	private buffer: number[];

	constructor() {
		this.reader = null;
		this.writer = null;
		this.port = null;
		this.messageHandler = null;
		this.isRunning = false;
		this.buffer = [];
	}

	public getPort(): any {
		return this.port;
	}

	public setMessageHandler(device: Device): void {
		this.messageHandler = new MessageHandler(this, device);
	}

	/**
	 * Open serial port, send ready broadcast, and start reading messages
	 */
	public async open(baudRate: number): Promise<number> {
		// @ts-ignore - Web Serial API
		this.port = await navigator.serial.requestPort();
		await this.port.open({ baudRate: baudRate });
		
		this.reader = this.port.readable.getReader();
		this.writer = this.port.writable.getWriter();
		this.isRunning = true;
		
		console.log('Port opened, sending ready broadcast: F0 7D 7F 7F 00 00 00 F7');
		
		// Send ready broadcast
		const readyMsg = MMM_Msg.create(0x0000, parseInt(CONST.SYSEX_DeviceReady, 16));
		await this.sendMessage(readyMsg);
		
		// Start reading loop
		this.startReading();
		
		// Wait for first DeviceReady response to get device ID
		return await this.waitForDeviceReady();
	}

	/**
	 * Wait for device ready message and extract device ID
	 */
	private async waitForDeviceReady(): Promise<number> {
		const timeout = 5000;
		const startTime = Date.now();
		
		return new Promise((resolve, reject) => {
			const checkInterval = setInterval(() => {
				if (Date.now() - startTime > timeout) {
					clearInterval(checkInterval);
					reject(new Error('Timeout waiting for device ready'));
				}
			}, 100);
			
			// Temporarily override message handler callback
			const originalHandler = this.messageHandler;
			const tempCallback = async (msg: MMM_Msg) => {
				if (msg.Type() === parseInt(CONST.SYSEX_DeviceReady, 16)) {
					clearInterval(checkInterval);
					const deviceId = msg.Source();
					console.log(`Device ready from ID: 0x${deviceId.toString(16).padStart(4, '0')}`);
					
					// Pass to original handler too
					if (originalHandler) {
						await originalHandler.processMessage(msg);
					}
					
					resolve(deviceId);
				} else if (originalHandler) {
					// Pass other messages to handler
					await originalHandler.processMessage(msg);
				}
			};
			
			// Use temp callback for first message
			(this as any).tempMessageCallback = tempCallback;
		});
	}

	/**
	 * Start continuous reading loop
	 */
	private async startReading(): Promise<void> {
		while (this.isRunning && this.reader) {
			try {
				const { value, done } = await this.reader.read();
				
				if (done) {
					console.log('Reader closed');
					break;
				}
				
				if (value) {
					// Process each byte
					for (const byte of value) {
						this.buffer.push(byte);
						
						// Check for complete message (ends with F7)
						if (this.buffer.length >= 8 && byte === 0xF7) {
							await this.processBuffer();
						}
						
						// Prevent buffer overflow
						if (this.buffer.length > 1024) {
							console.warn('Buffer overflow, clearing');
							this.buffer = [];
						}
					}
				}
			} catch (error) {
				if (this.isRunning) {
					console.error('Read error:', error);
					await this.sleep(100);
				}
			}
		}
	}

	/**
	 * Process accumulated buffer as a message
	 */
	private async processBuffer(): Promise<void> {
		// Check if valid SysEx message
		if (this.buffer[0] === 0xF0 && this.buffer[1] === CONST.SYSEX_ID) {
			const msgBytes = new Uint8Array(this.buffer);
			const hexStr = Array.from(msgBytes)
				.map(b => b.toString(16).padStart(2, '0').toUpperCase())
				.join(' ');
			console.log('Received:', hexStr);
			
			const msg = MMM_Msg.parse(msgBytes);
			
			if (msg) {
				// Check for temp callback (during initial handshake)
				if ((this as any).tempMessageCallback) {
					await (this as any).tempMessageCallback(msg);
					delete (this as any).tempMessageCallback;
				} else if (this.messageHandler) {
					// Route to message handler
					await this.messageHandler.processMessage(msg);
				}
			} else {
				console.error('Failed to parse message');
			}
		}
		
		// Clear buffer
		this.buffer = [];
	}

	/**
	 * Close connection and cleanup
	 */
	public async close(): Promise<void> {
		console.log('Closing connection...');
		this.isRunning = false;
		
		if (this.messageHandler) {
			this.messageHandler.destroy();
			this.messageHandler = null;
		}
		
		try {
			if (this.reader) {
				await this.reader.cancel();
				this.reader.releaseLock();
				this.reader = null;
			}
			if (this.writer) {
				this.writer.releaseLock();
				this.writer = null;
			}
			if (this.port) {
				await this.port.close();
				this.port = null;
			}
		} catch (error) {
			console.error('Close error:', error);
		}
		
		console.log('Connection closed');
	}

	/**
	 * Send a message
	 */
	public async sendMessage(msg: MMM_Msg): Promise<void> {
		await this.sendHexByteArray(msg.toBytes());
	}

	/**
	 * Send hex string
	 */
	public async sendHexString(hexString: string): Promise<void> {
		hexString = hexString.replace(/\s/g, '').toUpperCase();
		const bytes = new Uint8Array(hexString.length / 2);
		for (let i = 0; i < bytes.length; i++) {
			bytes[i] = parseInt(hexString.substr(i * 2, 2), 16);
		}
		await this.sendHexByteArray(bytes);
	}

	/**
	 * Send byte array
	 */
	public async sendHexByteArray(data: Uint8Array | number[]): Promise<void> {
		if (!this.writer) {
			console.error('No writer available');
			return;
		}
		
		try {
			// Ensure data is a Uint8Array
			const byteArray = data instanceof Uint8Array ? data : new Uint8Array(data);
			
			await this.writer.ready;
			await this.writer.write(byteArray);
			
			const hexStr = Array.from(byteArray)
				.map(b => b.toString(16).padStart(2, '0').toUpperCase())
				.join(' ');
			console.log('Sent:', hexStr);
		} catch (error) {
			console.error('Send error:', error);
		}
	}

	private sleep(ms: number): Promise<void> {
		return new Promise(resolve => setTimeout(resolve, ms));
	}
}
