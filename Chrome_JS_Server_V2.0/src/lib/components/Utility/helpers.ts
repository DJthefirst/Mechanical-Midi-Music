let MidiNotes = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];

declare global {
	interface String {
		padZero(length: number): string;
		toHex(): Uint8Array;
		toAsciiString(): string;
	}
	interface Number {
		toHexString(): string;
		toMidiNote(): string;
	}
}

//Pads 0s to String
String.prototype.padZero = function (length: number) {
	let str = String(this);
	while (str.length < length) {
		str = '0' + str;
	}
	return str;
};

// Converts a HexString to Uint8Array
String.prototype.toHex = function () {
	let str = String(this);
	let hexBytes = new Uint8Array(Math.ceil(str.length / 2));
	for (let i = 0; i < hexBytes.length; i++)
		hexBytes[i] = parseInt(str.substring(i * 2, i * 2 + 2), 16);
	return hexBytes;
};

// Converts a String to ASCII String
String.prototype.toAsciiString = function () {
	let str = String(this);
	let result = '';
	for (let i = 0; i < str.length; i++) {
		result += str.charCodeAt(i).toHexString();
	}
	return result;
};

//Converts Number to Paded Hex String
Number.prototype.toHexString = function () {
	let num = Number(this);
	return num.toString(16).padZero(2);
};

//Converts Midi Note Number to a Musical Note String
Number.prototype.toMidiNote = function () {
	let num = Number(this);
	return MidiNotes[num % 12] + String(Math.floor(num / 12) - 1);
};

export {};
