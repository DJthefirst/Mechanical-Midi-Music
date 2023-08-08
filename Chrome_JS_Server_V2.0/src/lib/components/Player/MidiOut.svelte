<script lang="ts">

	import JZZ from 'jzz'; // @ts-ignore
	import MidiOutElem from './MidiOutElem.svelte';

	async function loadOutputs(){
		let outputs = JZZ().info().outputs;
		for(let output of outputs)console.log('MidiOut: '+output['name']);

		let ports = [];
		for (let i=0; i<4; i++){
			let port = await JZZ().openMidiOut(i);
			ports.push(port);
		}
		return(ports);
	}

</script>

<div>
	<h1 class="text-center font-bold italic">MIDI OUT</h1>
	<ul class="outline outline-dark-50 overflow-y-scroll
    h-16 rounded-sm"
	>
		{#await loadOutputs()}
			<li class = ''>-Loaing outputs-</li>	
		{:then midiOutputs}
			{#each midiOutputs as port}
				<MidiOutElem port={port}/>
			{/each} 
		{:catch error}
			<p style="color: red">{error.message}</p>
		{/await}
	</ul>
</div>
