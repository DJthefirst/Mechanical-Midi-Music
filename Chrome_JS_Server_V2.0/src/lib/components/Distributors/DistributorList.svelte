<script lang="ts">
	import checkmark from '$lib/images/checkmark.svg';
	import { distributorListStore } from '$lib/store/stores';
	import type { DistributorList } from './Distributor';

	$: curDistributorId = 0;
	$: console.log(curDistributorId);

	let DistributionMethodType = [
		"StriaghtThrough",
		"RoundRobin",
		"RoundRobinBalance",
		"Ascending",
		"Descending",
		"Stack"
	]

	let distributorList : DistributorList;
	distributorListStore.subscribe((prev_value: any) => distributorList = prev_value);

	// let distributor1 = new Distributor(0x0001, 0x0000000f, 1, 0, 127, 1, false, false, false);
	// let distributor2 = new Distributor(0x0002, 0x000000f0, 10, 0, 127, 1, false, true, true);
	// let distributor3 = new Distributor(0x0004, 0x00000f00, 100, 0, 127, 1, true, true, true);
	// let distributor4 = new Distributor(0x0008, 0x0000f000, 1000, 0, 127, 1, true, false, false);
</script>

<div class="div-outline overflow-y-auto">
	<p class="text-center font-bold">Distributor List</p>
	<div class="h-full">
		{#each distributorList as distributor}
			<!-- svelte-ignore a11y-no-static-element-interactions -->
			<div
				class="{distributor.getId() == curDistributorId
					? 'bg-gray-select'
					: 'bg-gray-dark hover:bg-gray-700'} 
                    m-2 rounded-xl flex flex-row flex-wrap justify-start select-none cursor-pointer"
				on:click={() => (curDistributorId = distributor.getId())}
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
