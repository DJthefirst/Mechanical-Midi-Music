import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import Network from "./Network.js";

let midiSourceDropDown = document.getElementById("midiSourcesDropDown");

export default class MidiControler {
    constructor(){
        this.midiSourcesAvail = [];
        this.midiOutputsAvail = [];
        this.midiSourcesSel = [];
        this.midiOutputsSel = [];
        this.midiAccess = null;

        this.initMidiPorts();
        midiSourceDropDown.onchange = () => this.updateSelectedMidiSource();
    }

    async initMidiPorts(){
        await navigator.requestMIDIAccess()
        .then((midiAccess) => {
            this.midiAccess = midiAccess;
            this.updateMidiSources();
            midiAccess.onstatechange = (event) => this.portStateChange(event.port);
        });
    }

    updateMidiSources() {
        for(let port of this.midiAccess.inputs.values()){
            if (this.midiSourcesAvail.indexOf(port.id) == -1) {
                this.midiSourcesAvail.push(port.id);
                
                let option = document.createElement("option");
                option.setAttribute('value', port);
                option.setAttribute('id', "midi-" + port.id);

                let optionText = document.createTextNode(port.name);
                option.appendChild(optionText);
                    
                midiSourceDropDown.appendChild(option);
                port.open();
            }
        } 
    }

    portStateChange(port) {
        console.log('"' + port.name + '" has ' + port.state);
        if( port.state == "disconnected" ){
            let option = document.getElementById("midi-" + port.id);
            midiSourceDropDown.removeChild(option);
            port.close();
        }
        this.updateMidiSources();
    }

    updateSelectedMidiSource(){
        //Remove Listeners Here
        this.midiSourcesSel = [];
        for(let option of midiSourceDropDown.options){
            if (option.value == ""){ continue }
            if (option.selected) {
                for (let port of this.midiAccess.inputs.values()){
                    if (option.id == ("midi-" + port.id)){
                        this.midiSourcesSel.push(port);
                        port.onmidimessage = (event) => {this.onMidiMessage(event.data)}; 
                        console.log(port);
                        break;
                    }
                }
            }
        }
        console.log("Selected Midi Source(s): " + this.midiSourcesSel);
    }

    onMidiMessage(message){
        for(let device of DeviceList.getDeviceList()){
            Network.SendHexByteArray(device.getPort(), message);
        }
    }
    
}