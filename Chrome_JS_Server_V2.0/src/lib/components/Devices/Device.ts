import type { Connection } from "../Utility/ComManager";
import type { Distributor } from "../Distributors/Distributor";
import type ComManager from "../Utility/ComManager";
import { comManagerStore } from "$lib/store/stores";

let comManager: ComManager;
comManagerStore.subscribe((prev_value) => comManager = prev_value);

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
		this.platform = platform;
		this.deviceType = deviceType;
		this.numInstruments = numInstruments;
		this.noteMin = noteMin;
		this.noteMax = noteMax;
		this.distributors = [];
	}

	public getConnection(){
		return this.connection;
	}

	public syncDevice(){
		comManager.syncDevice(this);
	}

	public syncDistributors(){
		comManager.syncDistributors(this);
	}
}
