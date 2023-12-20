<script lang="ts">
	import { selectedDeviceStore, selectedDistributorStore } from '$lib/store/stores';
	import { Distributor } from './Distributor';

	let selectedDistributionMethod: number;
	let distributionMethods = [
		{ value: 0, method: 'StriaghtThrough'},
		{ value: 1, method: 'RoundRobin'},
		{ value: 2, method: 'RoundRobinBalance'},
		{ value: 3, method: 'Ascending'},
		{ value: 4, method: 'Descending'},
		{ value: 5, method: 'Stack'}
	
	];

	$: formChannels = '1-4';
	$: formInstruments = '1-4';
	$: formDistributionMethod = 2;
	$: formDamperEnable = false;
	$: formPolyphonicEnable = false;
	$: formNoteOverwrite = false;
	$: formNoteMin = 0;
	$: formNoteMax = 127;
	$: formNumPolyphonicNotes = 1;

	// Save Distributor to Device
	async function saveDistributor(addDistributor: boolean) {
		if ($selectedDistributorStore === undefined) addDistributor = true;

		console.log($selectedDistributorStore.getMuted())

		let distributor = new Distributor(
			listedItemsToNumber(formChannels),
			listedItemsToNumber(formInstruments),
			formDistributionMethod,
			formNoteMin,
			formNoteMax,
			formNumPolyphonicNotes,
			addDistributor ? false : $selectedDistributorStore.getMuted(), //Muted
			formDamperEnable,
			formPolyphonicEnable,
			formNoteOverwrite
		);
		console.log(distributor)

		if (addDistributor) distributor.setId($selectedDeviceStore.getDistributors().length);
		else distributor.setId($selectedDistributorStore.getId());
		$selectedDistributorStore = distributor;
		await $selectedDeviceStore.saveDistributor(distributor).then(() => {
			$selectedDeviceStore = $selectedDeviceStore;
		});
	}

	function listedItemsToNumber(input: any) {
		const RegExp_NumRanges = /(\d+)\s?-\s?(\d+)/g;
		const RegExp_Num = /(?<=^|[^-\s\d]|\d\s)\s*(\d+)\s*(?=[^-\s\d]|\s\d|$)/g;

		let result = 0x00;
		if (typeof input === 'number') return input;
		if (typeof input !== 'string') return result;

		//If hex binary or epression return computation
		if (/0x|0b|[|&]/gi.test(input)) {
			return eval(input);
		}

		//Else convet item list to Number
		let num;
		let numRange;

		//Match all Indivudal itmes "1,2,4"
		while ((num = RegExp_Num.exec(input)) !== null) {
			if ((result & (0x01 << (Number(num[1]) - 1))) === 0)
				// If number not added.
				result += 0x01 << (Number(num[1]) - 1);
		}

		//Match all Item Ranges "1-3"
		while ((numRange = RegExp_NumRanges.exec(input)) !== null) {
			for (let i = Number(numRange[1]); i <= Number(numRange[2]); i++) {
				if ((result & (0x01 << (i - 1))) === 0)
					// If number not added.
					result += 0x01 << (i - 1);
			}
		}

		return result;
	}

	function NumberToListedItems(num: number) {
		let result = '';
		let i = 0;

		let prev = false;
		let cur = ((num >> i) & 0x01) != 0;
		while (num > 0) {
			let next = ((num >> (i + 1)) & 0x01) != 0;

			let curValue = String(i + 1);

			if (cur) {
				if (!prev) result += curValue;
				if (!prev && !next) result += ',';
				if (prev && !next) result += curValue + ',';
				if (!prev && next) result += '-';
				num = num ^ (0x01 << i);
			}

			prev = cur;
			cur = next;
			i++;
		}
		return result.slice(0, -1);
	}

	async function removeDistributor() {
		$selectedDeviceStore.removeDistributor($selectedDistributorStore).then(() => {
			$selectedDeviceStore = $selectedDeviceStore;
			$selectedDistributorStore =
				$selectedDeviceStore.getDistributors()[$selectedDistributorStore.getId()];
		});
	}

	async function clearDistributor() {
		$selectedDeviceStore.clearDistributors().then(() => {
			$selectedDeviceStore = $selectedDeviceStore;
			$selectedDistributorStore =
				$selectedDeviceStore.getDistributors()[$selectedDistributorStore.getId()];
		});
	}

	// Update Form on Selected Distributor Change
	$: if ($selectedDistributorStore !== undefined) updateForm();
	function updateForm() {
		formChannels = NumberToListedItems($selectedDistributorStore.channels);
		formInstruments = NumberToListedItems($selectedDistributorStore.instruments);
		formDistributionMethod = $selectedDistributorStore.distributionMethod;
		formNoteMin = $selectedDistributorStore.minNote;
		formNoteMax = $selectedDistributorStore.maxNote;
		formNumPolyphonicNotes = $selectedDistributorStore.maxPolypnonic;
		formDamperEnable = $selectedDistributorStore.damper;
		formPolyphonicEnable = $selectedDistributorStore.polyphonic;
		formNoteOverwrite = $selectedDistributorStore.noteOverwrite;
	}
</script>

<div class="div-outline">
	<p class="text-center font-bold">Distributor Manager</p>
	<div class="flex justify-start flex-wrap">
		<div>
			<label for="channels" class="font-semibold pl-4">Channels</label>
			<input
				bind:value={formChannels}
				type="text"
				id="channels"
				placeholder="0x000F"
				class="bg-gray-dark rounded-md px-3 py-1 m-3 w-32 font-semibold"
			/>
		</div>
		<div>
			<label for="instruments" class="font-semibold pl-4">Instruments</label>
			<input
				bind:value={formInstruments}
				type="text"
				id="instruments"
				placeholder="0x0000000F"
				class="bg-gray-dark rounded-md px-3 py-1 m-3 w-48 font-semibold"
			/>
		</div>
	</div>
	<div>
		<label for="distributionMethodSelect" class="font-semibold my-2 ml-4">Distribution Method</label>
		<select bind:value={formDistributionMethod} class="w-64 rounded-md m-3 px-4 py-1 bg-gray-dark">
			{#each distributionMethods as distributionMethod}
				<option value={distributionMethod.value}>{distributionMethod.method}</option>
			{/each}
		</select>
	</div>
	<div class="flex justify-start flex-wrap">
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Damper Enable</label>
			<input bind:checked={formDamperEnable} type="checkbox" id="deviceName" class="bg-gray-dark" />
		</div>
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Polyphonic Enable</label>
			<input
				bind:checked={formPolyphonicEnable}
				type="checkbox"
				id="deviceName"
				class="bg-gray-dark"
			/>
		</div>
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Note Overwrite</label>
			<input
				bind:checked={formNoteOverwrite}
				type="checkbox"
				id="deviceName"
				class="bg-gray-dark"
			/>
		</div>
	</div>
	<div>
		<label for="noteMinMax" class="font-semibold pl-4">Note Min/Max</label>
		<input
			bind:value={formNoteMin}
			type="text"
			id="noteMinMax"
			placeholder="0"
			class="bg-gray-dark rounded-md px-3 py-1 my-3 w-24 font-semibold"
		/>
		<span class="font-extrabold">-</span>
		<input
			bind:value={formNoteMax}
			type="text"
			id="noteMinMax"
			placeholder="127"
			class="bg-gray-dark rounded-md px-3 py-1 my-3 w-24 font-semibold"
		/>
	</div>
	<div>
		<label for="numPolyphonicNotes" class="font-semibold pl-4">Number of Polyphonic Notes</label>
		<input
			bind:value={formNumPolyphonicNotes}
			type="text"
			id="numPolyphonicNotes"
			placeholder="1"
			class="bg-gray-dark rounded-md px-3 py-1 m-3 w-24 font-semibold"
		/>
	</div>
	<div class="flex justify-center m-2">
		<button on:click={() => saveDistributor(false)} class="button-player-green mx-2"
			>Update Distributor</button
		>
		<button on:click={() => saveDistributor(true)} class="button-player-green mx-2"
			>Add Distributor</button
		>
		<button on:click={() => removeDistributor()} class="button-player-red mx-2"
			>Remove Distributor</button
		>
		<button on:click={() => clearDistributor()} class="button-player-red mx-2"
			>Clear Distributors</button
		>
	</div>
</div>
