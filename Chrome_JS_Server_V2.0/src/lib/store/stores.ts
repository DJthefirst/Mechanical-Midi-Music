import { writable } from 'svelte/store';
import { midiNodeStore } from "./midiStore";

export const comPortsStore = writable();