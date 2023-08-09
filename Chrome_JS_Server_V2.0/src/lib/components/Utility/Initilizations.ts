import { midiNodeStore } from "$lib/store/stores";
import ComManager from "./ComManager";
import { comManagerStore } from "$lib/store/stores";

let midiNode: any;
let comManager = new ComManager;

comManagerStore.set(comManager);

midiNodeStore.subscribe((prev_value) => midiNode = prev_value);
comManagerStore.subscribe((prev_value) => comManager = prev_value);

midiNode._receive = function(msg: any) { 
    this.emit(msg); 
    console.log(msg.reverse());
    comManager.sendMidiToDevices(msg.reverse());
};

export function startMidi(){
    midiNodeStore.set(midiNode);
}