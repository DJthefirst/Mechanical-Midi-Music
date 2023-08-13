import { writable } from 'svelte/store';
import type { Device } from '../components/Devices/Device';
import { DistributorList } from '$lib/components/Distributors/Distributor';
import type ComManager from '$lib/components/Utility/ComManager';

import JZZ from 'jzz';
let midiNode = JZZ.Widget(); // @ts-ignore 

export const midiNodeStore = writable(midiNode);
export const deviceListStore = writable<Device[]>([]);
export const distributorListStore = writable<DistributorList>(new DistributorList);
export const comManagerStore = writable<ComManager>();