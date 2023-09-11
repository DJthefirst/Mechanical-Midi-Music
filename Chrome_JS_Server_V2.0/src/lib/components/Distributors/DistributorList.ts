import { selectedDistributorStore } from '$lib/store/stores';
import type { Distributor } from './Distributor';

let selectedDistributor: Distributor;
selectedDistributorStore.subscribe((prev_val) => (selectedDistributor = prev_val));

export class DistributorList {
	private list: Distributor[] = [];

	constructor() {}

	// define the default iterator for this class
	*[Symbol.iterator]() {
		for (let i = 0; i < this.list.length; i++) {
			yield this.list[i];
		}
	}

	// add for each iterator
	forEach(callback: any) {
		for (let data of this) callback(data);
	}

	public append(elem: Distributor) {
		elem.setId(this.list.length);
		this.list.push(elem);
		if (selectedDistributor === undefined) selectedDistributorStore.set(elem);
	}

	public pop(id: number) {
		let elem = this.list.splice(id, 1);
		for (let i = id; i < this.list.length; i++) {
			this.list[i].decId();
		}
		//@ts-ignore
		if (selectedDistributor.getId() == id) selectedDistributorStore.set(undefined);
		return elem;
	}
}
