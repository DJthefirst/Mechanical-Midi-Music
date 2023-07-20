<script>
import MidiIn from './MidiIn.svelte';
import MidiOut from './MidiOut.svelte';
import MidiPlayer from './MidiPlayer.svelte';

</script>

<div class="div-outline">
    <p class = "text-center font-bold">MIDI Player</p>
    <div class = 'flex min-h-96 flex-row justify-end'>
        <div class = 'p-2 w-full'>
            <MidiPlayer />
        </div>
        <div class = 'flex flex-col justify-evenly mr-2 w-56 min-w-56'>
            <div class = 'flex flex-col justify-center m-0'>
                <button class = 'button-player-green w-full'><i class="material-icons">add</i> Add Song</button>
                <button class = 'button-player-green w-full'><i class="material-icons">playlist_add</i> Add Playlist</button>
                <button class = 'button-player-red w-full'><i class="material-icons">remove</i> Remove Song</button>
                <button class = 'button-player-red w-full'><i class="material-icons">playlist_remove</i> Clear Playlist</button>
            </div>
            <div class = 'flex flex-col justify-evenly'>
                <MidiIn />
                <MidiOut />
            </div>
        </div>
    </div>
</div>