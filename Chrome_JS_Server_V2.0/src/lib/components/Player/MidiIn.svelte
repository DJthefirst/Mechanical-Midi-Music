<script lang="ts">
	import { onMount } from 'svelte';
	import JZZ from 'jzz'; // @ts-ignore
	import MidiInElem from './MidiInElem.svelte';

	// Detect Midi Changes
	$: midiPorts = [];

	//Set event trigger
	onMount(async function initMidiPorts(){
		await navigator.requestMIDIAccess()
		.then((midiAccess) => {
			midiAccess.onstatechange = () => updateInputs();
			updateInputs();
		});
	});

	async function updateInputs(){
		await JZZ()

		let ports = [];
		let inputs = JZZ().info().inputs;
		for(let input of inputs){
			let port = JZZ().openMidiIn(input['name']);
			ports.push(port);
		}
		midiPorts = ports;
	}

</script>

<div>
	<h1 class="text-center font-bold italic">MIDI IN</h1>
	<ul class="outline outline-dark-50 overflow-y-scroll
    h-16 rounded-sm"
	>
		{#each midiPorts as port}
			<MidiInElem port={port}/>
		{/each} 
	</ul>
</div>
