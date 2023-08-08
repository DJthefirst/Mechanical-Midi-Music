<script lang="ts">

	import JZZ from 'jzz'; // @ts-ignore
	import MidiInElem from './MidiInElem.svelte';

	async function loadInputs(){
		let inputs = JZZ().info().inputs;
		for(let input of inputs)console.log('MidiIn: '+input['name']);

		let ports = [];
		for (let i=0; i<4; i++){
			let port = await JZZ().openMidiIn(i);
			ports.push(port);
		}
		return(ports);
	}

</script>

<div>
	<h1 class="text-center font-bold italic">MIDI IN</h1>
	<ul class="outline outline-dark-50 overflow-y-scroll
    h-16 rounded-sm"
	>
		{#await loadInputs()}
			<li class = ''>-Loading Inputs-</li>	
		{:then midiInputs}
			{#each midiInputs as port}
				<MidiInElem port={port}/>
			{/each} 
		{:catch error}
			<p style="color: red">{error.message}</p>
		{/await}
	</ul>
</div>
