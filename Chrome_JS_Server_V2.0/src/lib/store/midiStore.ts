import { writable } from 'svelte/store';
import JZZ from 'jzz';

export const midiNodeStore = writable();

let midiNode = JZZ.Widget(); // @ts-ignore 
midiNode._receive = function(msg) { this.emit(msg); };
midiNodeStore.set(midiNode);