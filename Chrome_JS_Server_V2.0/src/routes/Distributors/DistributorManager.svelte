<div class="div-outline">
	<p class="text-center font-bold">Distributor Manager</p>
	<div class="flex justify-start flex-wrap">
		<div>
			<label for="channels" class="font-semibold pl-4">Channels</label>
			<input
				type="text"
				id="channels"
				placeholder="0x000F"
				class="bg-gray-dark rounded-md px-3 py-1 m-3 w-32 font-semibold"
			/>
		</div>
		<div>
			<label for="instruments" class="font-semibold pl-4">Instruments</label>
			<input
				type="text"
				id="instruments"
				placeholder="0x0000000F"
				class="bg-gray-dark rounded-md px-3 py-1 m-3 w-48 font-semibold"
			/>
		</div>
	</div>
	<div class="flex justify-start flex-wrap">
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Omni Mode Enable</label>
			<input type="checkbox" id="deviceName" class="bg-gray-dark" />
		</div>
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Omni Mode Enable</label>
			<input type="checkbox" id="deviceName" class="bg-gray-dark" />
		</div>
		<div>
			<label for="deviceName" class="font-semibold my-2 ml-4">Omni Mode Enable</label>
			<input type="checkbox" id="deviceName" class="bg-gray-dark" />
		</div>
	</div>
	<div>
		<label for="noteMinMax" class="font-semibold pl-4">Note Min/Max</label>
		<input
			type="text"
			id="noteMinMax"
			placeholder="0"
			class="bg-gray-dark rounded-md px-3 py-1 my-3 w-24 font-semibold"
		/>
		<span class="font-extrabold">-</span>
		<input
			type="text"
			id="noteMinMax"
			placeholder="127"
			class="bg-gray-dark rounded-md px-3 py-1 my-3 w-24 font-semibold"
		/>
	</div>
	<div>
		<label for="numPolyphonicNotes" class="font-semibold pl-4">Number of Polyphonic Notes</label>
		<input
			type="text"
			id="numPolyphonicNotes"
			placeholder="1"
			class="bg-gray-dark rounded-md px-3 py-1 m-3 w-24 font-semibold"
		/>
	</div>
	<div class="flex justify-center m-2">
		<button class="button-player-green mx-2">Update Device</button>
		<button class="button-player-green mx-2">Add Device</button>
		<button class="button-player-red mx-2">Remove Device</button>
		<button class="button-player-red mx-2">Clear Device</button>
	</div>
</div>
