import type { Connection } from "../Utility/ComManager";
import { Distributor } from "../Distributors/Distributor";
import type ComManager from "../Utility/ComManager";
import { comManagerStore, deviceListStore } from "$lib/store/stores";
import type DeviceList from "./DeviceList.svelte";

let comManager: ComManager;
let deviceList: DeviceList | never[];
comManagerStore.subscribe((prev_value) => comManager = prev_value); // @ts-ignore 
deviceListStore.subscribe((prev_value) => deviceList = prev_value);

export class Device {
	private connection: Connection;
	private distributors: Distributor[]; 
	public id: number;
	public name: string;
	public version: string;
	public platform: string;
	public deviceType: string;
	public numInstruments: number;
	public noteMin: number;
	public noteMax: number;

	constructor(
		connection: Connection,
		id: number,
		name: string,
		version: string,
		platform: string,
		deviceType: string,
		numInstruments: number,
		noteMin: number,
		noteMax: number
	) {
		this.connection = connection;
		this.id = id;
		this.name = name;
		this.version = version;
		this.platform = 'platform';
		this.deviceType = deviceType;
		this.numInstruments = 0;
		this.noteMin = 0;
		this.noteMax = 127;
		this.distributors = [];
	}

	public getConnection(){
		return this.connection;
	}

	public getDistributors(){
		return this.distributors;
	}

	public syncDevice(){
		comManager.syncDevice(this);
	}

	public update(array: Uint8Array){
		this.id = (array[0] + array[1]);
		this.deviceType = String(array[3]);
		this.version = String(array[4] << 7 + array[5]); //@ts-ignore
		this.name = String.fromCharCode(...array.slice(6,25)); //@ts-ignore
		deviceListStore.set(deviceList);
	}

	public updateDistributor(array: Uint8Array){
		let id = (array[0] << 7) + (array[1]);
		let channels = ((array[2] << 14) + (array[3] << 7) + array[4]);
		let instruments = ((array[5] << 28) + (array[6] << 21) + (array[7] << 14) + (array[8] << 7) + array[9]);
		let distributionMethod = array[10];
		let minNote = array[12];
		let maxNote = array[13];
		let maxPolypnonic = array[14];
		let damper = (array[15] & 0b00000001) != 0;
		let polyphonic = (array[15] & 0b00000010) != 0;
		let noteOverwrite = (array[15] & 0b00000100) != 0;

		let distributor = new Distributor(channels, instruments, distributionMethod, 
			minNote, maxNote, maxPolypnonic, damper, polyphonic, noteOverwrite)

		if (this.distributors.length < id) // Add Protection
			this.distributors.push(distributor);
		else this.distributors[id] = distributor;
	}

	public syncDistributors(){
		comManager.syncDistributors(this);
	}
}
