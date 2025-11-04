<script lang="ts">
	import { onMount } from 'svelte';
	import { browser } from '$app/environment';
	import JZZ from 'jzz'; // @ts-ignore
	import MidiInElem from './MidiInElem.svelte';

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
				updateInputs();
			};
			
			await updateInputs();
		} catch (error) {
			console.error('[MidiIn] Failed to get MIDI access:', error);
		}
	});

	async function updateInputs() {
		if (!browser) {
			return;
		}
		
		try {
			await JZZ();

			const info = JZZ().info();
			let ports = [];
			let inputs = info.inputs;
			
			for (let input of inputs) {
				let port = JZZ().openMidiIn(input['name']);
				ports.push(port);
			}
			
			midiPorts = ports;
		} catch (error) {
			console.error('[MidiIn] Failed to update MIDI inputs:', error);
		}
	}
	
	// Workaround for onMount not firing in some SvelteKit configurations
	if (browser && typeof window !== 'undefined') {
		setTimeout(async () => {
			if (!mounted) {
				mounted = true;
				try {
					if (!navigator.requestMIDIAccess) {
						console.error('[MidiIn] Web MIDI API not supported in this browser');
						return;
					}
					
					const midiAccess = await navigator.requestMIDIAccess();
					
					midiAccess.onstatechange = () => {
						updateInputs();
					};
					
					await updateInputs();
				} catch (error) {
					console.error('[MidiIn] Failed to get MIDI access:', error);
				}
			}
		}, 1000);
	}
</script>

<div>
	<h1 class="text-center font-bold italic">MIDI IN</h1>
	<ul
		class="outline outline-dark-50 overflow-y-scroll
    h-16 rounded-sm"
	>
		{#each midiPorts as port}
			<MidiInElem {port} />
		{/each}
	</ul>
</div>
