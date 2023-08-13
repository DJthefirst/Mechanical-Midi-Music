<script lang="ts">
	import checkmark from '$lib/images/checkmark.svg';
	import type { Device } from './Device';
	import { comManagerStore, distributorListStore } from '$lib/store/stores';
	import { deviceListStore } from '$lib/store/stores';
	import { DistributorList } from '../Distributors/Distributor';
	
	//distributorListStore.subscribe((prev_value: any) => distributorList = prev_value);

	let selectedBaudRate: number;
	let baudRates = [
		{value: 1200},
		{value: 2400},
		{value: 4800},
		{value: 9600},
		{value: 19200},
		{value: 38400},
		{value: 57600},
		{value: 115200},
		{value: 230400}
	];
	let curDevice: Device;
	//$: console.log(curDevice.id);

	function addDevice() {
		$comManagerStore.addDevice(selectedBaudRate);
	}

	function selectDevice(device: Device) {
		curDevice = device;

		let distributorList = new DistributorList();
		for( let distributor of device.getDistributors()) distributorList.append(distributor);
		$distributorListStore = distributorList;
	}

</script>

<div class="div-outline">
	<p class="text-center font-bold">Device List</p>
	<div class="h-full">
		<div class="flex flex-wrap justify-between bg-gray-dark m-2 rounded-xl">
			<select bind:value={selectedBaudRate} class="w-28 rounded-lg m-2 px-4 bg-gray-light">
				{#each baudRates as baudRate}
				<option value={baudRate.value}>{baudRate.value}</option>
				{/each}
			</select>
			<div class="flex flex-row">
				<button class="button-player-green m-2 p-2" 
					on:click={() => addDevice()}
					><i class="material-icons">add</i> Add Device</button
				>
				<button class="button-player-red my-2 mr-2 p-2"
					><i class="material-icons">remove</i> Remove Device</button
				>
			</div>
		</div>

		{#each $deviceListStore as device}
			<!-- svelte-ignore a11y-no-static-element-interactions -->
			<div
				class="{device === curDevice ? 'bg-gray-select' : 'bg-gray-dark hover:bg-gray-700'} 
                    m-2 rounded-xl flex flex-row flex-wrap justify-start select-none cursor-pointer"
				on:click={() => selectDevice(device)}
			>
				<div class="font-bold flex flex-row flex-wrap justify-between w-full my-2 mx-4">
					<label
						>ID:
						<span>{device.id}</span>
					</label>
					<span class="px-2">{device.name}</span>
					<span>{"Connection"}</span>
				</div>
				<div class="flex flex-col py-2">
					<label class="px-2 text-sm italic text-gray-400 font-extralight"
						>Version:
						<span class="">{device.version}</span>
					</label>
					<label class="px-2"
						>Platform:
						<span class="font-semibold">{device.platform}</span>
					</label>
					<label class="px-2"
						>Device Type:
						<span class="font-semibold">{device.deviceType}</span>
					</label>
					<label class="px-2"
						>Number of Instruments:
						<span class="font-semibold">{device.numInstruments}</span>
					</label>
					<label class="px-2"
						>Notes:
						<span class="font-semibold">{device.noteMin}-{device.noteMax}</span>
					</label>
				</div>
			</div>
		{/each}
	</div>
</div>
