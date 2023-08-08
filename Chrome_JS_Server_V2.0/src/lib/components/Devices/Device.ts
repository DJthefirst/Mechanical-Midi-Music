export class Device {
	public id: number;
	public name: string;
	public connection: string;
	public version: string;
	public platform: string;
	public deviceType: string;
	public numInstruments: number;
	public noteMin: number;
	public noteMax: number;

	constructor(
		id: number,
		name: string,
		connection: string,
		version: string,
		platform: string,
		deviceType: string,
		numInstruments: number,
		noteMin: number,
		noteMax: number
	) {
		this.id = id;
		this.name = name;
		this.connection = connection;
		this.version = version;
		this.platform = platform;
		this.deviceType = deviceType;
		this.numInstruments = numInstruments;
		this.noteMin = noteMin;
		this.noteMax = noteMax;
	}
}
