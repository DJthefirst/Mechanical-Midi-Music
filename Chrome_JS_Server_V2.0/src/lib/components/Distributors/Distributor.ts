import {} from '../Utility/helpers';
import * as CONST from '../Utility/Constants';

export class Distributor {

	private id: number;
	public channels: number;
	public instruments: number;
	public distributionMethod: number;
	public minNote: number;
	public maxNote: number;
	public muted: boolean;
	public noteOverwrite: boolean;

	constructor(
		channels: number,
		instruments: number,
		distributionMethod: number,
		minNote: number,
		maxNote: number,
		muted: boolean,
		noteOverwrite: boolean
	) {
		this.id = 0;
		this.channels = channels;
		this.instruments = instruments;
		this.distributionMethod = distributionMethod;
		this.minNote = minNote;
		this.maxNote = maxNote;
		this.muted = muted;
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
			(this.muted ? CONST.DISTRIBUTOR_BOOL_MASK.MUTED : 0) |
			(this.noteOverwrite ? CONST.DISTRIBUTOR_BOOL_MASK.NOTEOVERWRITE : 0);
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
		// Byte 11: Reserved
		construct += '00';
		// Byte 12: Min note
		construct += Number(this.minNote).toHexString();
		// Byte 13: Max note
		construct += Number(this.maxNote).toHexString();
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
		muted: boolean,
		noteOverwrite: boolean
	) {
		this.channels = channels;
		this.instruments = instruments;
		this.distributionMethod = distributionMethod;
		this.minNote = minNote;
		this.maxNote = maxNote;
		this.muted = muted;
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
