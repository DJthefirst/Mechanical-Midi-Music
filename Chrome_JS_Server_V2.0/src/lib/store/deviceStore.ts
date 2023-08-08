import { writable } from 'svelte/store';
import { Device } from '../components/Devices/Device';

export const midiNodeStore = writable();

let midiNode = JZZ.Widget(); // @ts-ignore 
midiNode._receive = function(msg) { this.emit(msg); };
midiNodeStore.set(midiNode);