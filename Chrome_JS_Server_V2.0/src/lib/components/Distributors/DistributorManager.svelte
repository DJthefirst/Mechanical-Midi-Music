<script lang="ts">
	import { selectedDeviceStore, selectedDistributorStore } from "$lib/store/stores";
	import { Distributor } from "./Distributor";
	import updateDistributorList from "./DistributorList.svelte"
	
	$: formChannels = 0x000F;
	$: formInstruments = 0x0000000F;
	$: formDistributionMethod = 1;
	$: formDamperEnable = false;
	$: formPolyphonicEnable = false;
	$: formNoteOverwrite = false;
	$: formNoteMin = 0;
	$: formNoteMax = 127;
	$: formNumPolyphonicNotes = 1;

	// Save Distributor to Device
	async function saveDistributor(addDistributor: boolean){
		if($selectedDistributorStore === undefined) addDistributor = true;

		let distributor = new Distributor(
			formChannels,
			formInstruments,
			formDistributionMethod,
			formNoteMin,
			formNoteMax,
			formNumPolyphonicNotes,
			formDamperEnable,
			formPolyphonicEnable,
			formNoteOverwrite
		);

		if (addDistributor) distributor.setId($selectedDeviceStore.getDistributors.length);
		else distributor.setId($selectedDistributorStore.getId());
		$selectedDistributorStore = distributor;
		await $selectedDeviceStore.saveDistributor(distributor).then(() => {
			$selectedDeviceStore = $selectedDeviceStore;
		});
	}

	function removeDistributor(){}

	function clearDistributor(){}

	// Update Form on Selected Distributir Change
	$: if (($selectedDistributorStore !== undefined))updateForm();
	function updateForm(){
		formChannels = $selectedDistributorStore.channels;
		formInstruments = $selectedDistributorStore.instruments;
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
	<div class="flex justify-start flex-wrap">
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Damper Enable</label>
			<input bind:checked={formDamperEnable} type="checkbox" id="deviceName" class="bg-gray-dark" />
		</div>
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Polyphonic Enable</label>
			<input bind:checked={formPolyphonicEnable} type="checkbox" id="deviceName" class="bg-gray-dark" />
		</div>
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Note Overwrite</label>
			<input bind:checked={formNoteOverwrite} type="checkbox" id="deviceName" class="bg-gray-dark" />
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
		<button on:click = {() => saveDistributor(false)} class="button-player-green mx-2">Update Distributor</button>
		<button on:click = {() => saveDistributor(true)} class="button-player-green mx-2">Add Distributor</button>
		<button on:click = {() => removeDistributor()} class="button-player-red mx-2">Remove Distributor</button>
		<button on:click = {() => clearDistributor()} class="button-player-red mx-2">Clear Distributors</button>
	</div>
</div>
