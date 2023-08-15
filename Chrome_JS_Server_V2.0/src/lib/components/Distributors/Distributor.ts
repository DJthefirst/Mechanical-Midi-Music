export class Distributor {
	private id: number;
	public channels: number;
	public instruments: number;
	public distributionMethod: number;
	public minNote: number;
	public maxNote: number;
	public maxPolypnonic: number;
	public damper: boolean;
	public polyphonic: boolean;
	public noteOverwrite: boolean;

	constructor(
		channels: number,
		instruments: number,
		distributionMethod: number,
		minNote: number,
		maxNote: number,
		maxPolypnonic: number,
		damper: boolean,
		polyphonic: boolean,
		noteOverwrite: boolean
	) {
		this.id = 0;
		this.channels = channels;
		this.instruments = instruments;
		this.distributionMethod = distributionMethod;
		this.minNote = minNote;
		this.maxNote = maxNote;
		this.maxPolypnonic = maxPolypnonic;
		this.damper = damper;
		this.polyphonic = polyphonic;
		this.noteOverwrite = noteOverwrite;
	}

	public getId() {
		return this.id;
	}

	public setId(id: number) {
		this.id = id;
	}

	public setDistributor(
		channels: number,
		instruments: number,
		distributionMethod: number,
		minNote: number,
		maxNote: number,
		maxPolypnonic: number,
		damper: boolean,
		polyphonic: boolean,
		noteOverwrite: boolean
	) {
		this.channels = channels;
		this.instruments = instruments;
		this.distributionMethod = distributionMethod;
		this.minNote = minNote;
		this.maxNote = maxNote;
		this.maxPolypnonic = maxPolypnonic;
		this.damper = damper;
		this.polyphonic = polyphonic;
		this.noteOverwrite = noteOverwrite;
	}

	public decId() {
		this.id--;
	}
}
