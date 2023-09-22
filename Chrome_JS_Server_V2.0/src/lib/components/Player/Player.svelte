<script lang="ts">
	import MidiIn from './MidiIn.svelte';
	import MidiOut from './MidiOut.svelte';
	import MidiPlayer from './MidiPlayer.svelte';

	$: files = null;
	let player: any;
	let fileInput: any;

	$: files, updateDB();
	function updateDB(){
		if (files == null) return;
		for (const file of files) player.addSong(file);
	}

</script>

<div class="div-outline">
	<p class="text-center font-bold">MIDI Player</p>
	<div class="flex min-h-96 flex-row justify-end select-none">
		<div class="p-2 w-full">
			<MidiPlayer bind:this={player} />
		</div>
		<div class="flex flex-col justify-between mr-2 mb-3 w-56 min-w-56">
			<div class="flex flex-col justify-center m-0">
				
				<input bind:this={fileInput} accept="audio/midi, audio/x-midi" bind:files multiple type="file" class="invisible w-0 h-0">
				<button class="button-player-green w-full" on:click={()=>fileInput.click()}
					><i class="material-icons">add</i> Add Songs</button
				>
				<button class="button-player-red w-full" on:click={player.removeSong}
					><i class="material-icons">remove</i> Remove Song</button
				>
				<button class="button-player-red w-full" on:click={player.clearPlayList}
					><i class="material-icons">playlist_remove</i> Clear Playlist</button
				>
			</div>
			<MidiIn />
			<MidiOut />
		</div>
	</div>
</div>
