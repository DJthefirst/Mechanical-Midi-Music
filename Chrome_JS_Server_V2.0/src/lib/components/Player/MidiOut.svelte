<script lang="ts">
	import { onMount } from 'svelte';
	import JZZ from 'jzz'; // @ts-ignore
	import MidiOutElem from './MidiOutElem.svelte';

	// Detect Midi Changes
	$: midiPorts = [];

	//Set event trigger
	onMount(async function initMidiPorts() {
		await navigator.requestMIDIAccess().then((midiAccess) => {
			midiAccess.onstatechange = () => updateOutputs();
			updateOutputs();
		});
	});

	async function updateOutputs() {
		await JZZ();

		let ports = [];
		let outputs = JZZ().info().outputs;
		for (let output of outputs) {
			let port = JZZ().openMidiOut(output['name']);
			ports.push(port);
		}
		//@ts-ignore
		midiPorts = ports;
	}
</script>

<div>
	<h1 class="text-center font-bold italic">MIDI OUT</h1>
	<ul class="outline outline-dark-50 overflow-y-scroll
    h-16 rounded-sm"
	>
		{#each midiPorts as port}
			<MidiOutElem {port} />
		{/each}
	</ul>
</div>
