import { writable } from 'svelte/store';
import { midiNodeStore } from "./midiStore";
import type { Device } from '../components/Devices/Device';
import SerialManager from '$lib/components/Utility/SerialManager';

export const deviceListStore = writable<Device[]>([]);
export const serialManagerStore = writable(new SerialManager);