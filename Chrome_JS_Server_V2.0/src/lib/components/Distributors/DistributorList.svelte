<script lang="ts">
	import checkmark from '$lib/images/checkmark.svg';
	import { distributorListStore, selectedDeviceStore, selectedDistributorStore } from '$lib/store/stores';
	import type { Device } from '../Devices/Device';
	import { DistributorList } from './DistributorList';

	let DistributionMethodType = [
		"StriaghtThrough",
		"RoundRobin",
		"RoundRobinBalance",
		"Ascending",
		"Descending",
		"Stack"
	]

	let prev_selectedDeviceStore: Device;

	//Update Distributor List
	$: { $selectedDeviceStore; updateDistributorList()}
	export function updateDistributorList(){
		let distributorList = new DistributorList();
		if ($selectedDeviceStore !== undefined) {
			for( let distributor of $selectedDeviceStore.getDistributors()) distributorList.append(distributor);
		}
		distributorListStore.set(distributorList);

		// On Device change update selected Disributor
		if ($selectedDeviceStore == prev_selectedDeviceStore) return;
		if ($selectedDeviceStore === undefined) { //@ts-ignore
			$selectedDistributorStore = undefined; return;
		}
		$selectedDistributorStore = $selectedDeviceStore.getDistributors()[0];
		prev_selectedDeviceStore = $selectedDeviceStore;
	}

</script>

<div class="div-outline overflow-y-auto">
	<p class="text-center font-bold">Distributor List</p>
	<div class="h-full">
		{#each $distributorListStore as distributor}
			<!-- svelte-ignore a11y-no-static-element-interactions -->
			<div
				class="{distributor.getId() == $selectedDistributorStore.getId()
					? 'bg-gray-select'
					: 'bg-gray-dark hover:bg-gray-700'} 
                    m-2 rounded-xl flex flex-row flex-wrap justify-start select-none cursor-pointer"
				on:click={() => ($selectedDistributorStore = distributor)}
			>
				<div class="flex flex-col p-2 w-60">
					<label>
						Distributor ID:
						<span class="font-semibold">{distributor.getId()}</span>
					</label>
					<label>
						Channels:
						<span class="font-semibold">0x{distributor.channels.toString(16).padZero(4).toUpperCase()}</span>
					</label>
					<label>
						Instruments:
						<span class="font-semibold">0x{distributor.instruments.toString(16).padZero(8).toUpperCase()}</span>
					</label>
					<label>
						Distribution Method:
						<span class="font-semibold">{DistributionMethodType[distributor.distributionMethod]}</span>
					</label>
					<label>
						Note Min/Max:
						<span class="font-semibold">{distributor.minNote}-{distributor.maxNote}</span>
					</label>
				</div>
				<div class="flex flex-col p-2">
					<div class="flex flex-row">
						<label class={distributor.damper ? 'mr-2' : 'text-gray-500'}> Damper </label>
						{#if distributor.damper}
							<img src={checkmark} alt="Checked" class="w-4" />
						{/if}
					</div>

					<div class="flex flex-row">
						<label class={distributor.polyphonic ? 'mr-2' : 'text-gray-500'}> Polyphonic </label>
						{#if distributor.polyphonic}
							<img src={checkmark} alt="Checked" class="w-4" />
						{/if}
					</div>

					<div class="flex flex-row">
						<label class={distributor.noteOverwrite ? 'mr-2' : 'text-gray-500'}>
							Note Overwrite
						</label>
						{#if distributor.noteOverwrite}
							<img src={checkmark} alt="Checked" class="w-4" />
						{/if}
					</div>
				</div>
			</div>
		{/each}
	</div>
</div>
