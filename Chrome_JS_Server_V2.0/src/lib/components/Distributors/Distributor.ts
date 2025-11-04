import {} from '../Utility/helpers';
import * as CONST from '../Utility/Constants';

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
	public vibrato: boolean;

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
		noteOverwrite: boolean,
		vibrato: boolean
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
		this.vibrato = vibrato;
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
			(this.muted ? CONST.DISTRIBUTOR_BOOL_MASK.MUTED : 0) |
			(this.polyphonic ? CONST.DISTRIBUTOR_BOOL_MASK.POLYPHONIC : 0) |
			(this.noteOverwrite ? CONST.DISTRIBUTOR_BOOL_MASK.NOTEOVERWRITE : 0) |
			(this.damper ? CONST.DISTRIBUTOR_BOOL_MASK.DAMPERPEDAL : 0) |
			(this.vibrato ? CONST.DISTRIBUTOR_BOOL_MASK.VIBRATO : 0);
		return to7BitStr(booleanValues, 2);
	}

	public getConstruct() {
		let construct = '';
		// Bytes 0-1: Distributor ID (14-bit)
		construct += to7BitStr(this.id, 2);
		// Bytes 2-4: Channels (16-bit, 3 bytes)
		construct += to7BitStr(this.channels, 3);
		// Bytes 5-9: Instruments (32-bit, 5 bytes)
		construct += to7BitStr(this.instruments, 5);
		// Byte 10: Distribution method
		construct += this.distributionMethod.toHexString();
		// Byte 11: Min note
		construct += Number(this.minNote).toHexString();
		// Byte 12: Max note
		construct += Number(this.maxNote).toHexString();
		// Byte 13: Number of polyphonic notes
		construct += Number(this.maxPolypnonic).toHexString();
		// Bytes 14-15: Boolean values (14-bit)
		construct += this.getBoolean();
		// Bytes 16-23: Reserved (8 bytes)
		construct += '0000000000000000';
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
		noteOverwrite: boolean,
		vibrato: boolean
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
		this.vibrato = vibrato;
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
