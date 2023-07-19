<script>
import MidiIn from './MidiIn.svelte';
import MidiOut from './MidiOut.svelte';
import MidiPlayer from './MidiPlayer.svelte';


</script>

<div class="div-outline">
    <p class = "text-center font-bold">MIDI Player</p>
    <div class = 'flex h-{96} flex-row justify-end'>
        <div class = 'div-outline w-full'>
            <MidiPlayer />
        </div>
        <div class = 'flex flex-col mx-2 w-56 min-w-56'>
            <button class = 'button-player-green'><i class="material-icons">add</i> Add Song</button>
            <button class = 'button-player-green'><i class="material-icons">playlist_add</i> Add Playlist</button>
            <button class = 'button-player-red'><i class="material-icons">remove</i> Remove Song</button>
            <button class = 'button-player-red'><i class="material-icons">playlist_remove</i> Clear Playlist</button>
            <MidiIn />
            <div class="h-2"></div>
            <MidiOut />
        </div>
    </div>
</div>