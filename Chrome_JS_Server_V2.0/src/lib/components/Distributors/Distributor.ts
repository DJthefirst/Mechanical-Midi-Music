import {} from '../Utility/helpers';

export class Distributor {

	private id: number;
	public channels: number;
	public instruments: number;
	public distributionMethod: number;
	public minNote: number;
	public maxNote: number;
	public maxPolypnonic: number;
	public muted: boolean;
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
		muted: boolean,
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
		this.muted = muted;
		this.damper = damper;
		this.polyphonic = polyphonic;
		this.noteOverwrite = noteOverwrite;
	}

	public getId() {
		return this.id;
	}

	public getIdStr() {
		return to7BitStr(this.id, 2);
	}

	public getMuted() {
		return this.muted;
	}

	public toggleMuted() {
		this.muted = !this.muted;
	}

	public getBoolean() {
		let booleanValues =
		(this.muted ? 0x01 : 0) |(this.damper ? 0x02 : 0) | (this.polyphonic ? 0x04 : 0) | (this.noteOverwrite ? 0x08 : 0);
		return booleanValues.toHexString();
	}

	public getConstruct() {
		let construct = '';
		construct += to7BitStr(this.id, 2);
		construct += to7BitStr(this.channels, 3);
		construct += to7BitStr(this.instruments, 5);
		construct += this.distributionMethod.toHexString();
		construct += this.getBoolean();
		construct += Number(this.minNote).toHexString();
		construct += Number(this.maxNote).toHexString();
		construct += Number(this.maxPolypnonic).toHexString();
		construct += '00';
		return construct;
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
		muted: boolean,
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
		this.muted = muted;
		this.damper = damper;
		this.polyphonic = polyphonic;
		this.noteOverwrite = noteOverwrite;
	}

	public decId() {
		this.id--;
	}
}

function to7BitStr(num: number, len: number) {
	let result = '';
	for (let i = 0; i < len; i++) {
		result = ((num >> (7 * i)) & 0b01111111).toHexString() + result;
	}
	return result;
}
