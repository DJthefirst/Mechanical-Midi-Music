import { writable } from 'svelte/store';
import type { Device } from '../components/Devices/Device';
import type { DistributorList } from '$lib/components/Distributors/DistributorList';
import type { Distributor } from '$lib/components/Distributors/Distributor';
import type ComManager from '$lib/components/Utility/ComManager';

import JZZ from 'jzz';
let midiNode = JZZ.Widget(); // @ts-ignore

// export const playlistStore = writable<Playlist>(new Playlist());
export const midiNodeStore = writable(midiNode);
export const selectedDeviceStore = writable<Device>(undefined);
export const selectedDistributorStore = writable<Distributor>(undefined);
export const deviceListStore = writable<Device[]>([]);
export const distributorListStore = writable<DistributorList>(undefined);
export const comManagerStore = writable<ComManager>(undefined);
