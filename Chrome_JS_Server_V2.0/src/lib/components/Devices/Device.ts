import type ComManager from '../Utility/ComManager';
import type { Connection } from '../Utility/ComManager';
import { Distributor } from '../Distributors/Distributor';
import { comManagerStore, deviceListStore, selectedDeviceStore } from '$lib/store/stores';
import SerialConnection from '../Utility/SerialConnection';

let comManager: ComManager;
let deviceList: Device[];

deviceListStore.subscribe((prev_value: any) => (deviceList = prev_value));
comManagerStore.subscribe((prev_value) => (comManager = prev_value));

export class Device {
	private connection: Connection;
	private distributors: Distributor[];
	public id: number;
	public isOnmiMode: boolean;
	public numInstruments: number;
	public numSubInstruments: number;
	public instrumentType: number;
	public platform: number;
	public noteMin: number;
	public noteMax: number;
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
		version: number,
		name: string
	) {
		this.connection = connection;
		this.id = id;
		(this.isOnmiMode = false), (this.numInstruments = numInstruments);
		this.numSubInstruments = numSubInstruments;
		this.instrumentType = instrumentType;
		this.platform = platform;
		this.noteMin = noteMin;
		this.noteMax = noteMax;
		this.version = version;
		(this.name = name), (this.distributors = []);
	}

	// Get Device Connection
	public getConnection() {
		return this.connection;
	}

	// Get List of Device Distributors
	public getDistributors() {
		return this.distributors;
	}

	public reset() {
		comManager.resetDevice(this);
	}

	// Removes This Device from GUI
	public remove() {
		disconnectDevice(this);
	}

	// Syncs GUI Device Construct From Device
	public sync() {
		comManager.syncDevice(this);
	}

	// Syncs GUI Distributor Construct From Device
	public syncDistributors() {
		comManager.syncDistributors(this);
	}

	// Saves Config to Device
	public save(name: string, isOmniMode: boolean) {
		this.name = name;
		this.isOnmiMode = isOmniMode;
		comManager.saveDevice(this);
	}

	// Save Distributor Config to device or adds a new Distributor by ID
	public async saveDistributor(distributor: Distributor) {
		await comManager.saveDistributor(this, distributor);
	}

	// Save Distributor Config to device or adds a new Distributor by ID
	public async saveDistributorBool(distributor: Distributor) {
		await comManager.saveDistributorBool(this, distributor);
	}

	// Updates DeviceStore from Message
	public update(array: Uint8Array) {
		this.id = (array[0] << 7) + array[1];
		this.numInstruments = array[3];
		this.numSubInstruments = array[4];
		this.instrumentType = array[5];
		this.platform = array[6];
		this.noteMin = array[7];
		this.noteMax = array[8];
		this.version = array[9] << (7 + array[10]);
		this.name = String.fromCharCode(...array.slice(12, 32).filter((val) => val !== 0)); //@ts-ignore
		deviceListStore.set(deviceList);
	}

	// Updates Device Distributors from Message
	public updateDistributor(array: Uint8Array) {
		let id = (array[0] << 7) + array[1];
		let channels = (array[2] << 14) + (array[3] << 7) + array[4];
		let instruments =
			(array[5] << 28) + (array[6] << 21) + (array[7] << 14) + (array[8] << 7) + array[9];
		let distributionMethod = array[10];
		let muted = (array[11] & 0b00000001) != 0;
		let damper = (array[11] & 0b00000010) != 0;
		let polyphonic = (array[11] & 0b00000100) != 0;
		let noteOverwrite = (array[11] & 0b00001000) != 0;
		let minNote = array[12];
		let maxNote = array[13];
		let maxPolypnonic = array[14];

		let distributor = new Distributor(
			channels,
			instruments,
			distributionMethod,
			minNote,
			maxNote,
			maxPolypnonic,
			muted,
			damper,
			polyphonic,
			noteOverwrite
		);

		if (id >= this.distributors.length)
			// Add Protection
			this.distributors.push(distributor);
		else this.distributors[id] = distributor;
	}

	public async removeDistributor(distributor: Distributor) {
		this.distributors = [];
		await comManager.removeDistributor(this, distributor);
	}

	public async clearDistributors() {
		await comManager.clearDistributors(this);
		this.distributors = [];
	}
}

// Connects Device from GUI
export async function connectDevice(baudRate: number) {
	let connection = new SerialConnection();
	let device = new Device(connection, 0, 0, 0, 0, 0, 0, 127, 0, 'Not Connected');
	await connection.open(baudRate);
	connection.getPort().ondisconnect = () => {
		device.remove();
	};
	await comManager.syncDevice(device).then(() => {
		deviceList.push(device);
		deviceListStore.set(deviceList);
	});
}

// Disconnects Device from GUI
export async function disconnectDevice(device: Device) {
	await device.getConnection().sendHexByteArray(new Uint8Array([0xff]));
	await device.getConnection().close();
	let index = deviceList.indexOf(device);
	if (index < 0) return; // if item is found
	deviceList.splice(index, 1);
	deviceListStore.set(deviceList); //@ts-ignore
	selectedDeviceStore.set(undefined);
}
