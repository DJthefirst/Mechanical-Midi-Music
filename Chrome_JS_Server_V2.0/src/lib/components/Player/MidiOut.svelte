<script lang="ts">
	import { onMount } from 'svelte';
	import { browser } from '$app/environment';
	import JZZ from 'jzz'; // @ts-ignore
	import MidiOutElem from './MidiOutElem.svelte';

	// Detect Midi Changes
	let midiPorts: any[] = [];
	let mounted = false;

	//Set event trigger
	onMount(async function initMidiPorts() {
		mounted = true;
		
		if (!browser) {
			return;
		}
		
		try {
			const midiAccess = await navigator.requestMIDIAccess();
			
			midiAccess.onstatechange = () => {
				updateOutputs();
			};
			
			await updateOutputs();
		} catch (error) {
			console.error('[MidiOut] Failed to get MIDI access:', error);
		}
	});

	async function updateOutputs() {
		if (!browser) {
			return;
		}
		
		try {
			await JZZ();

			const info = JZZ().info();
			let ports = [];
			let outputs = info.outputs;
			
			for (let output of outputs) {
				let port = JZZ().openMidiOut(output['name']);
				ports.push(port);
			}
			
			midiPorts = ports;
		} catch (error) {
			console.error('[MidiOut] Failed to update MIDI outputs:', error);
		}
	}
	
	// Workaround for onMount not firing in some SvelteKit configurations
	if (browser && typeof window !== 'undefined') {
		setTimeout(async () => {
			if (!mounted) {
				mounted = true;
				try {
					if (!navigator.requestMIDIAccess) {
						console.error('[MidiOut] Web MIDI API not supported in this browser');
						return;
					}
					
					const midiAccess = await navigator.requestMIDIAccess();
					
					midiAccess.onstatechange = () => {
						updateOutputs();
					};
					
					await updateOutputs();
				} catch (error) {
					console.error('[MidiOut] Failed to get MIDI access:', error);
				}
			}
		}, 1000);
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
