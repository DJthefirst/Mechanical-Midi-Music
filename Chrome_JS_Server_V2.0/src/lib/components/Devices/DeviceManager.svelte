<script lang="ts">
	import { deviceListStore, selectedDeviceStore } from "$lib/store/stores";
	import { Distributor } from "../Distributors/Distributor";
	
	
	$: formDeviceID = 0x0001;
	$: formDeviceName = "";
	$: formOmniModeEnable = false;

	function saveDevice(){
		if($selectedDeviceStore.id === 0) return;
		
		$selectedDeviceStore.save(
			formDeviceName.slice(0,20),
			formOmniModeEnable
		);
	}

	function addDevice(){
		// ToDo add Device
	}

	async function removeDevice(){
		await $selectedDeviceStore.remove();
	}

	async function clearDevices(){
		//@ts-ignore
		$selectedDeviceStore = undefined;

		// Disconect all connected Devices
		for await (let device of $deviceListStore){
			device.remove();
		}
	}

	// Update Form on Selected Device Change
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
		<button on:click={ () => saveDevice()} class="button-player-green mx-2">Update Device</button>
		<button on:click={ () => addDevice()} class="button-player-green mx-2">Add Device</button>
		<button on:click={ () => removeDevice()} class="button-player-red mx-2">Remove Device</button>
		<button on:click={ () => clearDevices()} class="button-player-red mx-2">Clear Devices</button>
	</div>
</div>
