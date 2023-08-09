import { midiNodeStore } from "$lib/store/stores";
import SerialManager from "./SerialManager";
import { serialManagerStore } from "$lib/store/stores";

let midiNode: any;
let serialManager = new SerialManager;

serialManagerStore.set(serialManager);

midiNodeStore.subscribe((prev_value) => midiNode = prev_value);
serialManagerStore.subscribe((prev_value) => serialManager = prev_value);

midiNode._receive = function(msg: any) { 
    this.emit(msg); 
    console.log(msg.reverse());
    serialManager.sendMidiToDevices(msg.reverse());
};

export function startMidi(){
    midiNodeStore.set(midiNode);
}