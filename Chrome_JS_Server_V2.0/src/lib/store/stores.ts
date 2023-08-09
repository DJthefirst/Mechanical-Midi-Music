import { writable } from 'svelte/store';
import type { Device } from '../components/Devices/Device';
import type SerialManager from '$lib/components/Utility/SerialManager';

import JZZ from 'jzz';
let midiNode = JZZ.Widget(); // @ts-ignore 

export const midiNodeStore = writable(midiNode);
export const deviceListStore = writable([]);
export const serialManagerStore = writable<SerialManager>();