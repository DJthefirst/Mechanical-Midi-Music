import {} from "../Utility/helpers"

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

	public getConstruct() {
		let booleanValues = (
			(this.damper ? 0x01 : 0) |
			(this.polyphonic ? 0x02 : 0) |
			(this.noteOverwrite ? 0x04 : 0)
		);

		let construct = "";
		construct += to7BitStr(this.id, 2);
		construct += to8BitStr(this.channels, 3);
		construct += to8BitStr(this.instruments, 5);
		construct += this.distributionMethod.toHexString();
		construct += booleanValues.toHexString();
		construct += Number(this.minNote).toHexString();
		construct += Number(this.maxNote).toHexString();
		construct += Number(this.maxPolypnonic).toHexString();
		construct += "00";
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

function to7BitStr(num: number, len: number){
	let result = ""
	for (let i = 0; i < len; i++){
    	result = ((num >> 8*i) & 0b01111111).toHexString() + result;
	}
    return(result);
}

function to8BitStr(num: number, len: number){
	let result = ""
	for (let i = 0; i < len; i++){
    	result = ((num >> 7*i)& 0b01111111).toHexString() + result;
	}
    return(result);
}