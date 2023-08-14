<script lang="ts">
	import { selectedDeviceStore } from "$lib/store/stores";
	import { Distributor } from "../Distributors/Distributor";
	
	
	$: formDeviceID = 0x0001;
	$: formDeviceName = "";
	$: formOmniModeEnable = false;

	function updateDevice(){
		$selectedDeviceStore.setDevice(
			formDeviceName.slice(0,20),
			formOmniModeEnable
		);
	}

	$: if (($selectedDeviceStore !== undefined) && $selectedDeviceStore.id !== 0)updateForm();
	function updateForm(){
		formDeviceID = $selectedDeviceStore.id;
		formDeviceName = $selectedDeviceStore.name;
		formOmniModeEnable = $selectedDeviceStore.isOnmiMode;
	}


</script>

<div class="div-outline">
	<p class="text-center font-bold">Device Manager</p>
	<div class="flex justify-start flex-wrap">
		<div>
			<label for="deviceId" class="font-semibold pl-4">Device ID</label>
			<input
				bind:value={formDeviceID}
				type="text"
				id="devId"
				placeholder="0x0001"
				class="bg-gray-dark rounded-md px-3 py-1 m-3 w-20 font-semibold"
			/>
		</div>
		<div>
			<label for="deviceName" class="font-semibold pl-4">Device Name</label>
			<input
				bind:value={formDeviceName}
				type="text"
				id="deviceName"
				placeholder=""
				class="bg-gray-dark rounded-md px-3 py-1 m-3 w-48 font-semibold"
			/>
		</div>
	</div>
	<div class="flex justify-start">
		<label for="deviceName" class="font-semibold m-2 ml-4">Omni Mode Enable</label>
		<input bind:checked={formOmniModeEnable} type="checkbox" id="deviceName" class="bg-gray-dark" />
	</div>
	<div class="flex justify-center m-2">
		<button on:click={ () => updateDevice()} class="button-player-green mx-2">Update Device</button>
		<button class="button-player-green mx-2">Add Device</button>
		<button class="button-player-red mx-2">Remove Device</button>
		<button class="button-player-red mx-2">Clear Device</button>
	</div>
</div>
