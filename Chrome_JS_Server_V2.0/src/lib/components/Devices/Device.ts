import type ComManager from '../Utility/ComManager';
import type { Connection } from '../Utility/ComManager';
import { Distributor } from '../Distributors/Distributor';
import { comManagerStore, deviceListStore, selectedDeviceStore } from '$lib/store/stores';
import SerialConnection from '../Utility/SerialConnection';
import * as CONST from '../Utility/Constants';
import { get } from 'svelte/store';

let comManager: ComManager;
let deviceList: Device[];

deviceListStore.subscribe((prev_value: any) => (deviceList = prev_value));
comManagerStore.subscribe((prev_value) => (comManager = prev_value));

export class Device {
	private connection: Connection;
	private distributors: Distributor[];
	public id: number;
	public muted: boolean;
	public isOnmiMode: boolean;
	public damperEnable: boolean;
	public vibratoEnable: boolean;
	public numInstruments: number;
	public numSubInstruments: number;
	public instrumentType: number;
	public platform: number;
	public noteMin: number;
	public noteMax: number;
	public numPolyphonicNotes: number;
	public version: number;
	public name: string;

	constructor(
		connection: Connection,
		id: number,
		numInstruments: number,
		numSubInstruments: number,
		instrumentType: number,
		platform: number,
		noteMin: number,
		noteMax: number,
		numPolyphonicNotes: number,
		version: number,
		name: string
	) {
		this.connection = connection;
		this.id = id;
		this.muted = false;
		this.isOnmiMode = false;
		this.damperEnable = false;
		this.vibratoEnable = false;
		this.numInstruments = numInstruments;
		this.numSubInstruments = numSubInstruments;
		this.instrumentType = instrumentType;
		this.platform = platform;
		this.noteMin = noteMin;
		this.noteMax = noteMax;
		this.numPolyphonicNotes = numPolyphonicNotes;
		this.version = version;
		this.name = name;
		this.distributors = [];
	}

	// Get Device Connection
	public getConnection(): Connection {
		return this.connection;
	}

	// Get List of Device Distributors
	public getDistributors(): Distributor[] {
		return this.distributors;
	}

	public reset(): void {
		comManager.resetDevice(this);
	}

	// Removes This Device from GUI
	public remove(): void {
		disconnectDevice(this);
	}

	// Syncs GUI Device Construct From Device
	public sync(): void {
		comManager.syncDevice(this);
	}

	// Syncs GUI Distributor Construct From Device
	public syncDistributors(): void {
		comManager.syncDistributors(this);
	}

	// Saves Config to Device
	public save(name: string, muted: boolean, isOmniMode: boolean, damperEnable: boolean, vibratoEnable: boolean): void {
		if (!comManager) {
			console.error('ComManager not initialized');
			return;
		}
		this.name = name;
		this.muted = muted;
		this.isOnmiMode = isOmniMode;
		this.damperEnable = damperEnable;
		this.vibratoEnable = vibratoEnable;
		comManager.saveDevice(this);
	}

	// Save Distributor Config to device or adds a new Distributor by ID
	public async saveDistributor(distributor: Distributor): Promise<void> {
		await comManager.saveDistributor(this, distributor);
	}

	// Save Distributor Boolean Config to device
	public async saveDistributorBool(distributor: Distributor): Promise<void> {
		await comManager.saveDistributorBool(this, distributor);
	}

	/**
	 * Updates Device configuration from received message payload
	 * Called by MessageHandler when device construct is received
	 */
	public update(array: Uint8Array): void {
		console.log('Parsing device construct, length:', array.length, 'bytes:', Array.from(array).map(b => b.toString(16).padStart(2, '0')).join(' '));
		
		// Parse device ID from response (bytes 0-1: 14-bit MSB, LSB)
		const receivedId = (array[0] << 7) + array[1];
		console.log('Received Device ID:', receivedId, '(0x' + receivedId.toString(16).padStart(4, '0') + ')');
		
		// Only update ID if it's valid and different
		if (!isNaN(receivedId) && receivedId !== 0) {
			if (this.id !== receivedId) {
				console.log('Updating device ID from', this.id, 'to', receivedId);
			}
			this.id = receivedId;
		}
		
		// Parse device configuration (56-byte struct)
		// Byte 2: Number of Instruments (7-bit)
		this.numInstruments = array[2];
		// Byte 3: Number of Sub Instruments (7-bit)
		this.numSubInstruments = array[3];
		// Byte 4: Reserved
		// Byte 5: Instrument Type (7-bit)
		this.instrumentType = array[5];
		// Byte 6: Reserved
		// Byte 7: Platform (7-bit)
		this.platform = array[7];
		// Byte 8: Reserved
		// Bytes 9-10: Firmware Version (14-bit MSB, LSB)
		this.version = (array[9] << 7) + array[10];
		// Byte 11: Reserved
		// Byte 12: Note Min (7-bit)
		this.noteMin = array[12];
		// Byte 13: Note Max (7-bit)
		this.noteMax = array[13];
		
		// Bytes 14-15: Boolean Values (14-bit MSB, LSB)
		const deviceBoolean = (array[14] << 7) + array[15];
		console.log('Device Boolean raw:', deviceBoolean, '(0x' + deviceBoolean.toString(16).padStart(4, '0') + ')');
		
		this.muted = (deviceBoolean & CONST.DEVICE_BOOL_MASK.MUTED) !== 0;
		this.isOnmiMode = (deviceBoolean & CONST.DEVICE_BOOL_MASK.OMNIMODE) !== 0;
		this.damperEnable = (deviceBoolean & CONST.DEVICE_BOOL_MASK.DAMPER) !== 0;
		this.vibratoEnable = (deviceBoolean & CONST.DEVICE_BOOL_MASK.VIBRATO) !== 0;
		console.log('Parsed: muted =', this.muted, ', omniMode =', this.isOnmiMode,
			', damper =', this.damperEnable, ', vibrato =', this.vibratoEnable);
		
		// Bytes 16-17: Reserved
		// Byte 18: Number of Polyphonic Notes (7-bit)
		this.numPolyphonicNotes = array[18];
		// Bytes 19-35: Reserved
		
		console.log('Config:', {
			numInstruments: this.numInstruments,
			numSubInstruments: this.numSubInstruments,
			instrumentType: this.instrumentType,
			platform: this.platform,
			noteMin: this.noteMin,
			noteMax: this.noteMax,
			numPolyphonicNotes: this.numPolyphonicNotes,
			version: this.version
		});
		
		// Parse device name (bytes 36-55: 20 ASCII bytes)
		this.name = String.fromCharCode(
			...array.slice(CONST.DEVICE_NAME_OFFSET, CONST.DEVICE_NAME_OFFSET + CONST.DEVICE_NUM_NAME_BYTES)
				.filter((val) => val !== 0)
		);
		console.log('Device name:', this.name);
		
		// Update device list store
		deviceListStore.set(deviceList);
	}

	/**
	 * Updates or adds a distributor from received message payload
	 * Called by MessageHandler when distributor construct is received
	 */
	public updateDistributor(array: Uint8Array): void {
		// Parse distributor ID (bytes 0-1: 14-bit MSB, LSB)
		const id = (array[0] << 7) + array[1];
		
		// Parse channels (bytes 2-4: 16-bit across 3 bytes)
		// Byte 2 contains bits 15-16 (MSB), Byte 3 is middle, Byte 4 is LSB
		const channels = (array[2] << 14) + (array[3] << 7) + array[4];
		
		// Parse instruments (bytes 5-9: 32-bit across 5 bytes)
		// Byte 5 contains bits 29-32 (MSB), then bytes 6-9
		const instruments =
			(array[5] << 28) + (array[6] << 21) + (array[7] << 14) + (array[8] << 7) + array[9];
		
		// Parse distribution method (byte 10)
		const distributionMethod = array[10];
		
		// Byte 11: Reserved
		
		// Parse note range (bytes 12-13)
		const minNote = array[12];
		const maxNote = array[13];
		
		// Parse boolean flags (bytes 14-15: 14-bit MSB, LSB)
		const booleanValue = (array[14] << 7) + array[15];
		const muted = (booleanValue & CONST.DISTRIBUTOR_BOOL_MASK.MUTED) !== 0;
		const noteOverwrite = (booleanValue & CONST.DISTRIBUTOR_BOOL_MASK.NOTEOVERWRITE) !== 0;

		const distributor = new Distributor(
			channels,
			instruments,
			distributionMethod,
			minNote,
			maxNote,
			muted,
			noteOverwrite
		);
		
		// Set the distributor ID
		distributor.setId(id);

		console.log('Updating distributor:', id, 'for device:', this.id, 'Total distributors before:', this.distributors.length);

		// Add or update distributor
		if (id >= this.distributors.length) {
			this.distributors.push(distributor);
		} else {
			this.distributors[id] = distributor;
		}
		
		console.log('Distributors after update:', this.distributors.length, this.distributors.map(d => d.getId()));
		
		// Force reactive update by creating new array reference for distributors
		this.distributors = [...this.distributors];
		
		// Update device in store to trigger Svelte reactivity
		const deviceList = get(deviceListStore);
		const index = deviceList.findIndex((d) => d.id === this.id);
		if (index !== -1) {
			deviceListStore.set([...deviceList]);
		}
		
		// Also trigger selectedDeviceStore update if this is the selected device
		const currentSelected = get(selectedDeviceStore);
		if (currentSelected && currentSelected.id === this.id) {
			selectedDeviceStore.set(this);
		}
	}

	/**
	 * Remove a specific distributor
	 */
	public async removeDistributor(distributor: Distributor): Promise<void> {
		await comManager.removeDistributor(this, distributor);
	}

	/**
	 * Clear all distributors
	 */
	public async clearDistributors(): Promise<void> {
		await comManager.clearDistributors(this);
		this.distributors = [];
	}

	/**
	 * Clear distributors array synchronously (used by MessageHandler)
	 */
	public clearDistributorsSync(): void {
		this.distributors = [];
	}
}

/**
 * Connect a new device via serial port
 * @param baudRate - Baud rate for serial communication
 */
export async function connectDevice(baudRate: number): Promise<void> {
	try {
		const connection = new SerialConnection();
		const device = new Device(connection, 0, 0, 0, 0, 0, 0, 127, 0, 0, 'Connecting...');
		
		// Set up message handler before opening connection
		connection.setMessageHandler(device);
		
		// Open connection and get device ID from ready handshake
		// The MessageHandler will automatically request device config when it receives DeviceReady
		const deviceId = await connection.open(baudRate);
		device.id = deviceId;
		console.log(`Device connected with ID: 0x${deviceId.toString(16).padStart(4, '0')}`);
		
		// Set up disconnect handler
		const port = connection.getPort();
		if (port) {
			port.ondisconnect = () => {
				console.log('Device disconnected');
				device.remove();
			};
		}
		
		// Add to device list
		deviceList.push(device);
		deviceListStore.set(deviceList);
		
		console.log('Device connection established successfully');
	} catch (error) {
		console.error('Failed to connect device:', error);
		throw error;
	}
}

/**
 * Disconnect a device
 * @param device - Device to disconnect
 */
export async function disconnectDevice(device: Device): Promise<void> {
	try {
		console.log('Disconnecting device:', device.id);
		
		// Send disconnect signal (0xFF)
		await device.getConnection().sendHexByteArray(new Uint8Array([0xff]));
		
		// Close connection
		await device.getConnection().close();
		
		// Remove from device list
		const index = deviceList.indexOf(device);
		if (index >= 0) {
			deviceList.splice(index, 1);
			deviceListStore.set(deviceList);
		}
		
		// Clear selected device if it was this one
		//@ts-ignore
		selectedDeviceStore.set(undefined);
		
		console.log('Device disconnected successfully');
	} catch (error) {
		console.error('Error disconnecting device:', error);
	}
}
