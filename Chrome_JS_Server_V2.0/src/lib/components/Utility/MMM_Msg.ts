import * as CONST from './Constants';

/**
 * MMM_Msg - Mechanical MIDI Music Message
 * Parses and represents a SysEx message in the MMM protocol
 * 
 * Message format: F0 7D [Source ID 14-bit] [Dest ID 14-bit] [Type] [Payload...] F7
 */
export class MMM_Msg {
	private source: number;      // Source device ID (14-bit)
	private destination: number; // Destination device ID (14-bit)
	private type: number;        // Message type
	private payload: Uint8Array; // Message payload

	constructor(source: number, destination: number, type: number, payload: Uint8Array) {
		this.source = source;
		this.destination = destination;
		this.type = type;
		this.payload = payload;
	}

	/**
	 * Parse a raw message buffer into an MMM_Msg object
	 * @param buffer - The raw message bytes including F0 7D ... F7
	 * @returns MMM_Msg object or null if parsing fails
	 */
	public static parse(buffer: Uint8Array): MMM_Msg | null {
		// Minimum message: F0 7D [2 bytes source] [2 bytes dest] [1 byte type] F7 = 8 bytes
		if (buffer.length < 8) {
			console.error('Message too short:', buffer.length);
			return null;
		}

		// Check for valid SysEx start and manufacturer ID
		if (buffer[0] !== 0xF0 || buffer[1] !== CONST.SYSEX_ID) {
			console.error('Invalid SysEx start:', buffer[0].toString(16), buffer[1].toString(16));
			return null;
		}

		// Check for valid SysEx end
		if (buffer[buffer.length - 1] !== 0xF7) {
			console.error('Invalid SysEx end:', buffer[buffer.length - 1].toString(16));
			return null;
		}

		// Parse source ID (14-bit, sent as 2 7-bit bytes: MSB LSB)
		const source = (buffer[2] << 7) | buffer[3];

		// Parse destination ID (14-bit, sent as 2 7-bit bytes: MSB LSB)
		const destination = (buffer[4] << 7) | buffer[5];

		// Parse message type
		const type = buffer[6];

		// Extract payload (everything between type and F7)
		const payload = buffer.slice(7, buffer.length - 1);

		return new MMM_Msg(source, destination, type, payload);
	}

	/**
	 * Parse a hex string into an MMM_Msg object
	 * @param hexString - Hex string representation of the message
	 * @returns MMM_Msg object or null if parsing fails
	 */
	public static parseHexString(hexString: string): MMM_Msg | null {
		// Remove spaces and convert to uppercase
		hexString = hexString.replace(/\s/g, '').toUpperCase();
		
		// Convert hex string to byte array
		const buffer = new Uint8Array(hexString.length / 2);
		for (let i = 0; i < buffer.length; i++) {
			buffer[i] = parseInt(hexString.substr(i * 2, 2), 16);
		}
		
		return MMM_Msg.parse(buffer);
	}

	/**
	 * Create a new message
	 * @param destination - Destination device ID (14-bit)
	 * @param type - Message type
	 * @param payload - Message payload (optional)
	 * @returns MMM_Msg object
	 */
	public static create(destination: number, type: number, payload?: Uint8Array): MMM_Msg {
		const serverID = 0x3FFF; // Server uses max 14-bit ID (0x3FFF)
		return new MMM_Msg(serverID, destination, type, payload || new Uint8Array(0));
	}

	/**
	 * Get the source device ID
	 */
	public Source(): number {
		return this.source;
	}

	/**
	 * Get the destination device ID
	 */
	public Destination(): number {
		return this.destination;
	}

	/**
	 * Get the message type
	 */
	public Type(): number {
		return this.type;
	}

	/**
	 * Get the payload bytes
	 */
	public Payload(): Uint8Array {
		return this.payload;
	}

	/**
	 * Convert message to hex string for transmission
	 * @returns Hex string representation
	 */
	public toHexString(): string {
		let result = 'F0' + CONST.SYSEX_ID.toString(16).toUpperCase().padStart(2, '0');
		
		// Source ID (14-bit as 2 7-bit bytes)
		result += ((this.source >> 7) & 0x7F).toString(16).toUpperCase().padStart(2, '0');
		result += (this.source & 0x7F).toString(16).toUpperCase().padStart(2, '0');
		
		// Destination ID (14-bit as 2 7-bit bytes)
		result += ((this.destination >> 7) & 0x7F).toString(16).toUpperCase().padStart(2, '0');
		result += (this.destination & 0x7F).toString(16).toUpperCase().padStart(2, '0');
		
		// Type
		result += this.type.toString(16).toUpperCase().padStart(2, '0');
		
		// Payload
		for (let byte of this.payload) {
			result += byte.toString(16).toUpperCase().padStart(2, '0');
		}
		
		// SysEx end
		result += 'F7';
		
		return result;
	}

	/**
	 * Convert message to byte array for transmission
	 * @returns Uint8Array representation
	 */
	public toBytes(): Uint8Array {
		const buffer = new Uint8Array(7 + this.payload.length + 1);
		let index = 0;
		
		// SysEx start and manufacturer ID
		buffer[index++] = 0xF0;
		buffer[index++] = CONST.SYSEX_ID;
		
		// Source ID (14-bit as 2 7-bit bytes)
		buffer[index++] = (this.source >> 7) & 0x7F;
		buffer[index++] = this.source & 0x7F;
		
		// Destination ID (14-bit as 2 7-bit bytes)
		buffer[index++] = (this.destination >> 7) & 0x7F;
		buffer[index++] = this.destination & 0x7F;
		
		// Type
		buffer[index++] = this.type;
		
		// Payload
		for (let byte of this.payload) {
			buffer[index++] = byte;
		}
		
		// SysEx end
		buffer[index++] = 0xF7;
		
		return buffer;
	}

	/**
	 * Get a string representation for debugging
	 */
	public toString(): string {
		return `MMM_Msg[Type: 0x${this.type.toString(16).padStart(2, '0')}, ` +
		       `Source: 0x${this.source.toString(16).padStart(4, '0')}, ` +
		       `Dest: 0x${this.destination.toString(16).padStart(4, '0')}, ` +
		       `PayloadLen: ${this.payload.length}]`;
	}
}
