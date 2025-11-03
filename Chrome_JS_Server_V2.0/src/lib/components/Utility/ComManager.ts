import type { Device } from '../Devices/Device';
import { deviceListStore } from '$lib/store/stores';
import * as CONST from './Constants';
import type { Distributor } from '../Distributors/Distributor';
import { MMM_Msg } from './MMM_Msg';

let deviceList: Device[];

deviceListStore.subscribe((prev_value: any) => (deviceList = prev_value));

export interface Connection {
	open: (baudRate: number) => Promise<number>;
	close: () => Promise<void>;
	sendHexString: (msg: string) => Promise<void>;
	sendHexByteArray: (msg: Uint8Array | number[]) => Promise<void>;
	sendMessage: (msg: MMM_Msg) => Promise<void>;
	getPort: () => any;
	setMessageHandler: (device: Device) => void;
}

/**
 * ComManager - Manages communication with devices
 * Provides high-level methods for device configuration and synchronization
 */
export default class ComManager {
	/**
	 * Send MIDI data to all connected devices
	 */
	public sendMidiToDevices(msg: Uint8Array): void {
		for (const device of deviceList) {
			device.getConnection().sendHexByteArray(msg);
		}
	}

	////////////////////////////////////////////////
	// Device Configuration Commands
	////////////////////////////////////////////////

	/**
	 * Save device configuration (name and boolean settings)
	 */
	public async saveDevice(device: Device): Promise<void> {
		console.log('Saving device configuration for device:', device.id);

				// Prepare device name (20 bytes ASCII)
		const nameBytes = new Uint8Array(20);
		for (let i = 0; i < Math.min(device.name.length, 20); i++) {
			nameBytes[i] = device.name.charCodeAt(i);
		}
				// Convert to hex string for sendCommand
		const nameHex = Array.from(nameBytes)
			.map(b => b.toString(16).padStart(2, '0'))
			.join('');
		
		// Prepare device boolean flags
		const deviceBoolean = 
			(device.muted ? CONST.DEVICE_BOOL_MASK.MUTED : 0) |
			(device.isOnmiMode ? CONST.DEVICE_BOOL_MASK.OMNIMODE : 0);

		// Send commands
		await this.sendCommand(device, CONST.SYSEX_SetDeviceName, nameHex);
		await this.sendCommand(device, CONST.SYSEX_SetDeviceBoolean, this.to14BitStr(deviceBoolean));
		
		// Sync to get updated configuration
		await this.syncDevice(device);
	}

	/**
	 * Reset device to default configuration
	 */
	public async resetDevice(device: Device): Promise<void> {
		console.log('Resetting device:', device.id);
		await this.sendCommand(device, CONST.SYSEX_ResetDeviceCfg, '');
		await this.syncDevice(device);
	}

	/**
	 * Synchronize device configuration from hardware
	 */
	public async syncDevice(device: Device): Promise<void> {
		console.log('Syncing device:', device.id);
		
		// Request device construct (without distributors)
		await this.sendCommand(device, CONST.SYSEX_GetDeviceConstructOnly, '');
		
		// Note: The response will be handled by MessageHandler
		// which will trigger device.update() and then request distributors
	}

	/**
	 * Synchronize all distributors from device
	 */
	public async syncDistributors(device: Device): Promise<void> {
		console.log('Syncing distributors for device:', device.id);
		
		// Request number of distributors
		await this.sendCommand(device, CONST.SYSEX_GetNumDistributors, '');
		
		// Note: MessageHandler will handle the response and request each distributor
	}

	////////////////////////////////////////////////
	// Distributor Configuration Commands
	////////////////////////////////////////////////

	/**
	 * Save a distributor configuration to the device
	 */
	public async saveDistributor(device: Device, distributor: Distributor): Promise<void> {
		console.log('Saving distributor for device:', device.id);
		await this.sendCommand(device, CONST.SYSEX_SetDistributor, distributor.getConstruct());
		await this.syncDevice(device);
	}

	/**
	 * Save distributor boolean settings
	 */
	public async saveDistributorBool(device: Device, distributor: Distributor): Promise<void> {
		console.log('Saving distributor boolean for device:', device.id);
		await this.sendCommand(
			device,
			CONST.SYSEX_SetDistributorID_Boolean,
			distributor.getIdStr() + distributor.getBoolean()
		);
		await this.syncDevice(device);
	}

	/**
	 * Remove a specific distributor from the device
	 */
	public async removeDistributor(device: Device, distributor: Distributor): Promise<void> {
		console.log('Removing distributor:', distributor.getId(), 'from device:', device.id);
		await this.sendCommand(device, CONST.SYSEX_RemoveDistributorID, this.to14BitStr(distributor.getId()));
		await this.syncDevice(device);
	}

	/**
	 * Clear all distributors from the device
	 */
	public async clearDistributors(device: Device): Promise<void> {
		console.log('Clearing all distributors from device:', device.id);
		await this.sendCommand(device, CONST.SYSEX_RemoveAllDistributors, '');
		await this.syncDevice(device);
	}

	////////////////////////////////////////////////
	// Helper Methods
	////////////////////////////////////////////////

	/**
	 * Send a SysEx command to a device
	 * @param device - Target device
	 * @param cmd - Command type (hex string)
	 * @param payload - Payload data (hex string or empty)
	 */
	private async sendCommand(device: Device, cmd: string, payload: string = ''): Promise<void> {
		const message = 
			CONST.SYSEX_START + 
			CONST.SYSEX_ServerID + 
			this.to14BitStr(device.id) + 
			cmd + 
			payload + 
			CONST.SYSEX_END;
		
		console.log('Sending command:', cmd, 'to device:', device.id);
		await device.getConnection().sendHexString(message);
	}

	/**
	 * Convert a number to 14-bit hex string (2 7-bit bytes)
	 * @param num - Number to convert
	 * @returns Hex string with MSB and LSB
	 */
	private to14BitStr(num: number): string {
		const lsb = (num & 0x7F).toString(16).padStart(2, '0').toUpperCase();
		const msb = ((num >> 7) & 0x7F).toString(16).padStart(2, '0').toUpperCase();
		return msb + lsb;
	}
}
