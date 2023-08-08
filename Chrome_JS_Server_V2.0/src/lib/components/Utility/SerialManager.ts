import { comPortsStore } from "$lib/store/stores";

let comPorts;
comPortsStore.subscribe((prev_value) => comPorts = prev_value);

function onMessage(){
    
}