<script lang="ts">
	import checkmark from '$lib/images/checkmark.svg';

	import { Device } from './Device';

	$: curdeviceId = 0;
	$: console.log(curdeviceId);

	let deviceList: Device[] = [];

	let device1 = new Device(1, 'Dulcimer', 'COM 3', '001.1', 'ESP32', 'Shift Register', 1, 0, 127);
	let device2 = new Device(
		2,
		'Air Compressor',
		'COM 4',
		'001.1',
		'Arduino Nano',
		'PWM Driver',
		8,
		0,
		127
	);
	let device3 = new Device(
		4,
		'Floppy Drives',
		'COM 6',
		'001.1',
		'Ardunio Uno',
		'Floppy Drive',
		4,
		0,
		127
	);
	let device4 = new Device(
		10,
		'Stepper Motor',
		'COM 7',
		'001.1',
		'Arduino Mega',
		'Stepper Motor',
		6,
		0,
		127
	);

	deviceList.push(device1);
	deviceList.push(device2);
	deviceList.push(device3);
	deviceList.push(device4);
</script>

<div class="div-outline">
	<p class="text-center font-bold">Device List</p>
	<div class="h-full">
		<div class="flex flex-wrap justify-between bg-gray-dark m-2 rounded-xl">
			<select class="w-28 rounded-lg m-2 px-4 bg-gray-light">
				<option value="1200"> 1200 </option>
				<option value="2400"> 2400 </option>
				<option value="4800"> 4800 </option>
				<option value="9600"> 9600 </option>
				<option value="19200"> 19200 </option>
				<option value="38400"> 38400 </option>
				<option value="57600"> 57600 </option>
				<option value="115200" selected> 115200 </option>
				<option value="230400"> 230400 </option>
			</select>
			<div class="flex flex-row">
				<button class="button-player-green m-2 p-2"
					><i class="material-icons">add</i> Add Device</button
				>
				<button class="button-player-red my-2 mr-2 p-2"
					><i class="material-icons">remove</i> Remove Device</button
				>
			</div>
		</div>

		{#each deviceList as device}
			<!-- svelte-ignore a11y-no-static-element-interactions -->
			<div
				class="{device.id == curdeviceId ? 'bg-gray-select' : 'bg-gray-dark hover:bg-gray-700'} 
                    m-2 rounded-xl flex flex-row flex-wrap justify-start select-none cursor-pointer"
				on:click={() => (curdeviceId = device.id)}
			>
				<div class="font-bold flex flex-row flex-wrap justify-between w-full my-2 mx-4">
					<label
						>ID:
						<span>{device.id}</span>
					</label>
					<span class="px-2">{device.name}</span>
					<span>{device.connection}</span>
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
