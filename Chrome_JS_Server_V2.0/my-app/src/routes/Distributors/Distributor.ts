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

    constructor(channels: number, instruments: number, distributionMethod: number,
        minNote: number, maxNote: number, maxPolypnonic: number,
        damper: boolean, polyphonic: boolean, noteOverwrite: boolean) {
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

    public getId(){
        return this.id;
    } 

    public setId(id: number){
        this.id = id;
    } 

    public decId(){
        this.id--;
    } 
}

export class DistributorList {
    private list : Distributor[] = [];

    constructor(){

    }

    // define the default iterator for this class
    * [Symbol.iterator]() {
        for (let i = 0; i < this.list.length; i++) {
            yield this.list[i];
        }
    }

    // add for each iterator
    forEach(callback: any) { 
        for (let data of this) callback(data);
    }
    
    public append(elem: Distributor) {
        elem.setId(this.list.length);
        this.list.push(elem);
    }

    public pop(id: number){
        var elem = this.list.splice(id,1);
        for (var i = 10; i < this.list.length; i++) {
            this.list[i].decId();
        }
        return(elem);
    }
}