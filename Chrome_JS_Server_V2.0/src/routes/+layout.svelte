<script lang="ts">
	import '../app.css';
	import { dev } from '$app/environment';
	import { inject } from '@vercel/analytics';

	import JZZ from 'jzz'; // @ts-ignore
	import SMF from 'jzz-midi-smf';

	SMF(JZZ);

	// Connect to System Midi Sound Synth
	let midiout = JZZ().openMidiOut(['Microsoft GS Wavetable Synth', 'Apple DLS Synth']).or('MIDI-Out: Cannot open!')
	.and(function(){ console.log('MIDI-Out:', this.name()); })
	
	// Generate Common Node
	let midiNode = JZZ.Widget().connect(midiout); // @ts-ignore 
		midiNode._receive = function(msg) { this.emit(msg); };


	JZZ().openMidiIn().connect(midiout);
	
	//x.connect(midiNode);

	async function loadInputs(){
		let inputs = JZZ().info().inputs;
		for(let input of inputs)console.log('MidiIn: '+input['name']);

		let ports = [];
		for (let i=0; i<3; i++){
			let port = await JZZ().openMidiIn(i);
			ports.push(port);
		}
		return(ports);
	}

	async function loadOutputs(){
		let outputs = JZZ().info().outputs;
		for(let output of outputs)console.log('MidiOut: '+output['name']);

		let ports = [];
		for (let i=0; i<3; i++){
			let port = await JZZ().openMidiOut(i);
			ports.push(port);
		}

		return(ports);
	}

	function connectInput(port: any) {
		JZZ().close();
		//JZZ().openMidiIn().connect(midiNode);
		//JZZ().refresh()
		//port.connect(midiNode);
	}

	async function connectOutput(port: any) {
		midiNode.connect(port);
	}
</script>


<div class='flex flex-col'>
	<div class='flex flex-row justify-between p-4 m-2  rounded-2xl outline outline-gray-light outline-4'>
		<span>GitHub</span>
		<span>Mechanical Midi Music</span>
		<span>Wiki</span>
	</div>

	{#await loadInputs()}
		<div class = ''>-Loading Inputs-</div>	
	{:then midiInputs}
		{#each midiInputs as port}
			<div class = '' on:click={() => connectInput(port)}>-{port.info()['name']}</div>	
		{/each} 
	{:catch error}
		<p style="color: red">{error.message}</p>
	{/await}

	<div>--------------------</div>

	{#await loadOutputs()}
		<div class = ''>-Loading Outputs-</div>	
	{:then midiOutputs}
		{#each midiOutputs as port}
			<div class = '' on:click={() => connectOutput(port)}>-{port.info()['name']}</div>	
		{/each} 
	{:catch error}
		<p style="color: red">{error.message}</p>
	{/await}


	<slot />
</div>

