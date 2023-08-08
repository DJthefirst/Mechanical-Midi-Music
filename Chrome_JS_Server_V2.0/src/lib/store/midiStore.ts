import { writable } from 'svelte/store';
import JZZ from 'jzz';

export const midiNodeStore = writable();

let midiNode = JZZ.Widget(); // @ts-ignore 
midiNodeStore.set(midiNode);