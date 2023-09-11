import { distributorListStore, midiNodeStore } from '$lib/store/stores';
import ComManager from './ComManager';
import { comManagerStore } from '$lib/store/stores';
import { DistributorList } from '../Distributors/DistributorList';

let midiNode: any;
let comManager = new ComManager();

distributorListStore.set(new DistributorList());
comManagerStore.set(comManager);

midiNodeStore.subscribe((prev_value) => (midiNode = prev_value));
comManagerStore.subscribe((prev_value) => (comManager = prev_value));

midiNode._receive = function (msg: any) {
	this.emit(msg);
	console.log(msg.reverse());
	comManager.sendMidiToDevices(msg.reverse());
};

export function startMidi() {
	midiNodeStore.set(midiNode);
}
