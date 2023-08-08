import type { Connection } from "../Utility/SerialManager";

export class Device {
	private connection: Connection;
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
	}

	public getConnection() {
		return this.connection;
	}
}
