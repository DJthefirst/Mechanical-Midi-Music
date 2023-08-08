<script>
	import DeviceList from "$lib/components/Devices/DeviceList.svelte";
	import DeviceManager from "$lib/components/Devices/DeviceManager.svelte";
	import DistributorList  from "$lib/components/Distributors/DistributorList.svelte";
	import DistributorManager from "$lib/components/Distributors/DistributorManager.svelte";
	import Player from "$lib/components/Player/Player.svelte";


</script>

<div
	class="grid grid-cols-1 md:grid-cols-5 xl:grid-cols-7
grid-rows-1 md:grid-rows-2 md:h-screen"
>
	<div class="div-grid md:row-span-2 md:h-screen md:col-span-3 xl:col-span-3 overflow-auto">
		<div>
			<Player />
		</div>
		<div>
			<DeviceManager />
		</div>
		<div>
			<DistributorManager />
		</div>
	</div>
	<div
		class="div-grid md:col-start-4 xl:col-start-4 md:col-span-2
md:row-start-1 row-span-1 xl:row-auto xl:h-screen overflow-auto"
	>
		<DeviceList />
	</div>
	<div
		class="div-grid md:col-start-4 xl:col-start-6 md:col-span-2
md:row-start-2 row-span-1 xl:row-auto xl:h-screen overflow-auto"
	>
		<DistributorList />
	</div>
</div>
