<script lang="ts">

    import JZZ from 'jzz'; // @ts-ignore
    import { midiNodeStore } from '$lib/store/midiStore';

    export let port;

    let selected = false;
    let midiNode;

    // Generate Common Node
    midiNodeStore.subscribe((prev_val) => midiNode = prev_val)

    function connectOutput(port: any) {
        if (selected){
            midiNode.disconnect(port);
        }
        else{
            midiNode.connect(port);
        }
        selected = !selected;
    }
        
</script>

<li class = 'li-player-io {selected ? 'bg-gray-select hover:bg-gray-select': ''}' on:click={() => connectOutput(port)}>{port.info()['name']}</li>