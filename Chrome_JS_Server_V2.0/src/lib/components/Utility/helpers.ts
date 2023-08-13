declare global {
    interface String {
      padZero(length: number): string;
      toHex(): Uint8Array;
    }
    interface Number{
      toHexString(): string;
    }  
  }
  
  //Pads 0s to String
  String.prototype.padZero = function (length: number) {
    let str = String(this)
    while (str.length < length) {
      str = '0' + str;
    }
    return str;
  };

  String.prototype.toHex = function () {
    let str = String(this)
    let hexBytes = new Uint8Array(Math.ceil(str.length / 2));
    for (let i = 0; i < hexBytes.length; i++) hexBytes[i] = parseInt(str.substring(i*2, i*2 +2), 16);
    return hexBytes;
  };

  Number.prototype.toHexString = function () {
    let num = Number(this);
    return num.toString(16).padZero(2);
  }
  export {}