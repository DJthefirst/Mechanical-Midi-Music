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
	public isOnmiMode: boolean;
	public numInstruments: number;
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
		instrumentType: number,
		platform: number,
		noteMin: number,
		noteMax: number,
		version: number,
		name: string,
	) {
		this.connection = connection;
		this.id = id;
		this.isOnmiMode = false,
		this.numInstruments = numInstruments;
		this.instrumentType = instrumentType;
		this.platform = platform;
		this.noteMin = noteMin;
		this.noteMax = noteMax;
		this.version = version;
		this.name = name,
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

	public setDevice(name : string, isOmniMode: boolean){
		this.name = name;
		this.isOnmiMode = isOmniMode;
		comManager.setDevice(this);
	}

	public update(array: Uint8Array){
		this.id = (array[0] + array[1]);
		this.numInstruments = array[3];
		this.instrumentType = array[4];
		this.platform = array[5];
		this.noteMin = array[6];
		this.noteMax = array[7];
		this.version = array[8] << 7 + array[9];
		this.name = String.fromCharCode(...array.slice(10,30).filter((val) => (val !== 0))); //@ts-ignore
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
