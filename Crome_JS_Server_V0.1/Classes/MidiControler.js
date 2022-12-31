import Device from "./Device.js";
import DeviceList from "./DeviceList.js";
import SerialCom from "./SerialCom.js";

let midiSourceDropDown = document.getElementById("midiSourceDropDown");
let midiOutputDropDown = document.getElementById("midiOutputDropDown");

export default class MidiControler {
    constructor(){
        this.midiSourcesAvail = [];
        this.midiOutputsAvail = [];
        this.midiSourcesSel = [];
        this.midiOutputsSel = [];
        this.midiAccess = null;

        this.initMidiPorts();
        midiSourceDropDown.onchange = () => this.updateSelectedMidiSource();
        midiOutputDropDown.onchange = () => this.updateSelectedMidiOutput();
    }

    async initMidiPorts(){
        await navigator.requestMIDIAccess()
        .then((midiAccess) => {
            this.midiAccess = midiAccess;
            this.updateMidiOptions();
            midiAccess.onstatechange = (event) => this.updateMidiOptions();
        });
    }

    updateMidiOptions() {
        //Convert Midi Access Iteratior into an Array
        let midiSourcePorts = [];
        let midiOutputPorts = [];
        for(let port of this.midiAccess.inputs.values()){midiSourcePorts.push(port)}
        for(let port of this.midiAccess.outputs.values()){midiOutputPorts.push(port)}

        //Create Array of Added Ports
        let addedSourcePorts = midiSourcePorts.filter(e => !this.midiSourcesAvail.includes(e.id));
        let addedOutputPorts = midiOutputPorts.filter(e => !this.midiOutputsAvail.includes(e.id));

        //Create Array of Removed Ports
        let removedSourcePorts = this.midiSourcesAvail
            .filter(e => !midiSourcePorts.map((e) => e.id).includes(e));
        let removedOutputPorts = this.midiOutputsAvail
            .filter(e => !midiOutputPorts.map((e) => e.id).includes(e));

        //Remove Ports from Available Ports
        this.midiSourcesAvail = this.midiSourcesAvail
            .filter(e => midiSourcePorts.map((e) => e.id).includes(e));
        this.midiOutputsAvail = this.midiOutputsAvail
            .filter(e => midiOutputPorts.map((e) => e.id).includes(e));

        for(let port of addedSourcePorts){
            this.midiSourcesAvail.push(port.id);
                
            let option = document.createElement("option");
            option.setAttribute('value', port);
            option.setAttribute('id', "midiIn-" + port.id);

            let optionText = document.createTextNode(port.name);
            option.appendChild(optionText);
            
            midiSourceDropDown.appendChild(option);
            port.open();
        }

        for(let port of addedOutputPorts){
            this.midiOutputsAvail.push(port.id);
                
            let option = document.createElement("option");
            option.setAttribute('value', port);
            option.setAttribute('id', "midiOut-" + port.id);

            let optionText = document.createTextNode(port.name);
            option.appendChild(optionText);
            
            midiOutputDropDown.appendChild(option);
            port.open();
        }

        for(let portId of removedSourcePorts){
            let option = document.getElementById("midiIn-" + portId);
            if(option == null) continue;
            midiSourceDropDown.removeChild(option);
        }

        for(let portId of removedOutputPorts){
            let option = document.getElementById("midiOut-" + portId);
            if(option == null) continue;
            midiOutputDropDown.removeChild(option);
        }
    }

    updateSelectedMidiSource(){
        //Remove Listeners Here
        this.midiSourcesSel = [];
        for(let option of midiSourceDropDown.options){
            if (option.value == ""){ continue }
            if (option.selected) {
                for (let port of this.midiAccess.inputs.values()){
                    if (option.id == ("midiIn-" + port.id)){
                        this.midiSourcesSel.push(port);
                        port.onmidimessage = (event) => {this.onMidiMessage(event.data)}; 
                        break;
                    }
                }
            }
        }
        console.log("Selected Midi Source(s): " + this.midiSourcesSel);
    }

    updateSelectedMidiOutput(){
        //Remove Listeners Here
        this.midiOutputsSel = [];
        for(let option of midiOutputDropDown.options){
            if (option.value == ""){ continue }
            if (option.selected) {
                for (let port of this.midiAccess.outputs.values()){
                    if (option.id == ("midiOut-" + port.id)){
                        this.midiOutputsSel.push(port);
                        console.log(port);
                        break;
                    }
                }
            }
        }
        console.log("Selected Midi Source(s): " + this.midiSourcesSel);
    }

    onMidiMessage(message){
        console.log("onMidiMsg: " + message);
        for(let device of DeviceList.getDeviceList()){
            device.com.SendHexByteArray(message);
        }
        for(let port of this.midiOutputsSel){
            port.send(message);
        }
    }
}