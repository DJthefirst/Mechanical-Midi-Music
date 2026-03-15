import { MMM_Msg } from './MMM_Msg';
import * as CONST from './Constants';
import type SerialConnection from './SerialConnection';
import type { Device } from '../Devices/Device';

/**
 * MessageHandler - Processes incoming MMM protocol messages
 * Routes messages to appropriate handler functions based on message type
 */
export class MessageHandler {
	private connection: SerialConnection;
	private device: Device;

	constructor(connection: SerialConnection, device: Device) {
		this.connection = connection;
		this.device = device;
	}

	/**
	 * Process an incoming message and route to appropriate handler
	 * @param msg - The parsed MMM message
	 */
	public async processMessage(msg: MMM_Msg): Promise<void> {
		console.log('Processing message:', msg.toString());

		const type = msg.Type();

		switch (type) {
			// System Commands (0x00 - 0x0F)
			case parseInt(CONST.SYSEX_DeviceReady, 16):
				await this.handleDeviceReady(msg);
				break;

			case parseInt(CONST.SYSEX_ResetDeviceCfg, 16):
				await this.handleResetDeviceConfig(msg);
				break;

			case parseInt(CONST.SYSEX_DiscoverDevices, 16):
				await this.handleDiscoverDevices(msg);
				break;

			// Get/Set Device Commands (0x20 - 0x2F)
			// Incoming messages are always responses to Get requests (device sending data)
			case parseInt(CONST.SYSEX_DeviceConstructFull, 16):
				await this.handleGetDeviceConstructFull(msg);
				break;

			case parseInt(CONST.SYSEX_DeviceConstructOnly, 16):
				await this.handleGetDeviceConstructOnly(msg);
				break;

			case parseInt(CONST.SYSEX_DeviceID, 16):
				console.log('Received DeviceID response');
				break;

			case parseInt(CONST.SYSEX_DeviceName, 16):
				await this.handleGetDeviceName(msg);
				break;

			case parseInt(CONST.SYSEX_DeviceBoolean, 16):
				await this.handleGetDeviceBoolean(msg);
				break;

			// Distributor Management Commands (0x30 - 0x3F)
			case parseInt(CONST.SYSEX_GetNumDistributors, 16):
				await this.handleGetNumOfDistributors(msg);
				break;

			case parseInt(CONST.SYSEX_AddSetDistributor, 16):
				console.log('AddSetDistributor response');
				break;

			case parseInt(CONST.SYSEX_GetAllDistributors, 16):
				await this.handleGetAllDistributors(msg);
				break;

			case parseInt(CONST.SYSEX_RemoveDistributorID, 16):
				console.log('RemoveDistributorID response');
				break;

			case parseInt(CONST.SYSEX_RemoveAllDistributors, 16):
				console.log('RemoveAllDistributors response');
				break;

			case parseInt(CONST.SYSEX_ToggleMuteDistributorID, 16):
				console.log('ToggleMuteDistributorID response');
				break;

			// Get/Set Distributor Commands (0x40 - 0x4F)
			case parseInt(CONST.SYSEX_DistributorID_Construct, 16):
				await this.handleGetDistributorConstruct(msg);
				break;

			case parseInt(CONST.SYSEX_DistributorID_Channels, 16):
				console.log('DistributorID_Channels response');
				break;

			case parseInt(CONST.SYSEX_DistributorID_Instruments, 16):
				console.log('DistributorID_Instruments response');
				break;

			case parseInt(CONST.SYSEX_DistributorID_DistributionMethod, 16):
				console.log('DistributorID_DistributionMethod response');
				break;

			case parseInt(CONST.SYSEX_DistributorID_Boolean, 16):
				console.log('DistributorID_Boolean response');
				break;

			case parseInt(CONST.SYSEX_DistributorID_MinMaxNotes, 16):
				console.log('DistributorID_MinMaxNotes response');
				break;

			case parseInt(CONST.SYSEX_DistributorID_NumPolyphonicNotes, 16):
				console.log('DistributorID_NumPolyphonicNotes response');
				break;

			// Instrument Commands (0x50 - 0x5F)
			case parseInt(CONST.SYSEX_ResetAllInstruments, 16):
				console.log('ResetAllInstruments response');
				break;

			case parseInt(CONST.SYSEX_ResetInstrument, 16):
				console.log('ResetInstrument response');
				break;

			case parseInt(CONST.SYSEX_GetInstrumentNumActiveNotes, 16):
				console.log('GetInstrumentNumActiveNotes response');
				break;

			case parseInt(CONST.SYSEX_SetInstrumentDirectMessage, 16):
				console.log('SetInstrumentDirectMessage response');
				break;

			case parseInt(CONST.SYSEX_SetInstrumentNoteOn, 16):
				console.log('SetInstrumentNoteOn response');
				break;

			case parseInt(CONST.SYSEX_SetInstrumentNoteOff, 16):
				console.log('SetInstrumentNoteOff response');
				break;

			default:
				console.log('Unknown SysEx type:', type.toString(16));
				break;
		}
	}

	/**
	 * Handle DeviceReady message - device is ready for communication
	 */
	private async handleDeviceReady(msg: MMM_Msg): Promise<void> {
		const deviceId = msg.Source();
		console.log('Device ready from:', deviceId.toString(16), '(0x' + deviceId.toString(16).padStart(4, '0') + ')');
		// Request device construct (0x21 = DeviceConstructOnly)
		await this.sendMessage(deviceId, parseInt(CONST.SYSEX_DeviceConstructOnly, 16));
	}

	/**
	 * Handle ResetDeviceConfig message - device config was reset
	 */
	private async handleResetDeviceConfig(msg: MMM_Msg): Promise<void> {
		console.log('Device config reset confirmed');
		// Acknowledge by sending reset command back
		await this.sendMessage(msg.Source(), parseInt(CONST.SYSEX_ResetDeviceCfg, 16));
	}

	/**
	 * Handle DiscoverDevices message - send broadcast discovery
	 */
	private async handleDiscoverDevices(msg: MMM_Msg): Promise<void> {
		console.log('Broadcasting device discovery');
		await this.sendMessage(0x0000, parseInt(CONST.SYSEX_DiscoverDevices, 16)); // Broadcast
	}

	/**
	 * Handle GetDeviceConstructFull - device construct with distributors
	 */
	private async handleGetDeviceConstructFull(msg: MMM_Msg): Promise<void> {
		console.log('Received full device construct');
		this.device.update(msg.Payload());
		// Request number of distributors
		await this.sendMessage(msg.Source(), parseInt(CONST.SYSEX_GetNumDistributors, 16));
	}

	/**
	 * Handle DeviceConstructOnly - device construct without distributors
	 */
	private async handleGetDeviceConstructOnly(msg: MMM_Msg): Promise<void> {
		console.log('Received device construct');
		this.device.update(msg.Payload());
		// Request number of distributors
		await this.sendMessage(msg.Source(), parseInt(CONST.SYSEX_GetNumDistributors, 16));
	}

	/**
	 * Handle GetDeviceName message - device name received
	 */
	private async handleGetDeviceName(msg: MMM_Msg): Promise<void> {
		console.log('Received device name');
		const payload = msg.Payload();
		// Parse name from first 20 bytes
		const name = String.fromCharCode(...payload.slice(0, 20).filter(val => val !== 0));
		console.log('Device name:', name);
		// Update device name through device object
		// this.device.name = name; // This should be done through device.update()
	}

	/**
	 * Handle GetDeviceBoolean message - device boolean flags received
	 */
	private async handleGetDeviceBoolean(msg: MMM_Msg): Promise<void> {
		console.log('Received device boolean');
		// Parse boolean flags
		const payload = msg.Payload();
		if (payload.length >= 2) {
			const deviceBoolean = (payload[0] << 7) | payload[1];
			console.log('Device boolean:', deviceBoolean.toString(16));
		}
	}

	/**
	 * Handle GetNumOfDistributors message - number of distributors received
	 */
	private async handleGetNumOfDistributors(msg: MMM_Msg): Promise<void> {
		const payload = msg.Payload();
		if (payload.length < 1) {
			console.error('Invalid GetNumOfDistributors payload');
			return;
		}

		const numDistributors = payload[0];
		console.log('Device has', numDistributors, 'distributors');
		
		// Clear existing distributors
		this.device.clearDistributorsSync();
		
		// Request each distributor construct
		for (let i = 0; i < numDistributors; i++) {
			const idPayload = new Uint8Array(2);
			idPayload[0] = (i >> 7) & 0x7F; // MSB
			idPayload[1] = i & 0x7F;        // LSB
			await this.sendMessage(msg.Source(), parseInt(CONST.SYSEX_DistributorID_Construct, 16), idPayload);
		}
	}

	/**
	 * Handle GetAllDistributors message - all distributors received
	 */
	private async handleGetAllDistributors(msg: MMM_Msg): Promise<void> {
		console.log('Received all distributors');
		// TODO: Implement parsing of all distributors in one message
	}

	/**
	 * Handle GetDistributorConstruct message - distributor info received
	 */
	private async handleGetDistributorConstruct(msg: MMM_Msg): Promise<void> {
		console.log('Received distributor construct');
		this.device.updateDistributor(msg.Payload());
	}

	/**
	 * Send a message to a device
	 * @param destination - Destination device ID
	 * @param type - Message type
	 * @param payload - Optional payload
	 */
	private async sendMessage(
		destination: number,
		type: number,
		payload?: Uint8Array
	): Promise<void> {
		const msg = MMM_Msg.create(destination, type, payload);
		console.log('Sending message:', msg.toString());
		await this.connection.sendMessage(msg);
	}

	/**
	 * Cleanup when handler is destroyed
	 */
	public destroy(): void {
		console.log('MessageHandler destroyed for device:', this.device.id);
		// Perform any cleanup needed
	}
}
