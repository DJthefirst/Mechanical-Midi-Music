var currentDevId = 0;

export class Device {

    constructor(port, ComType) {
      this.port = port;
      this.comType = ComType;
      this.instrumentType = 'Instrument';
      this.id = currentDevId;
      this.div = null;
      currentDevId++;

      /*this.config = null; */
      this.getConfig();

      this.createDiv();

    }

    getPort () {
        return this.port;
    }


    getId () {
        return this.id;
    }

    getDiv () {
        return this.div;
    }

    selectDevice () {
        console.log('Selected Device: ' + this.id);
        Array.from(document.getElementsByClassName('deviceDiv'))
            .forEach(function(element) {element.style.backgroundColor = "#5086ad"});
        document.getElementById('deviceDiv' + this.id).style.backgroundColor = 'white';
    }

    createDiv () {
        this.div = document.createElement('div');
        this.div.id = 'deviceDiv' + this.id;
        
        this.div.className = 'deviceDiv';
        this.div.appendChild(elementFromHTML(' \
            <div class="deviceInfo"> \
                <b class="deviceInfo">Dev: '+ this.id +'</b>\
                <b class="deviceInfo"> ' + this.instrumentType + '</b>\
                <b class="deviceInfo">' + this.comType +'</b>\
            </div> \
            <div> \
            </div> \
        '));

        document.getElementById('windowRightBar').append(this.div);
    }

    getConfig () {

    }
}

export class DevicePool{

    constructor (){
        this.devices = new Array();
        this.selectedDevice = null;
    }

    selectDevice1 (device) {
        this.selectedDevice = device;
        device.selectDevice();
    }

    add (device) {
        device.getDiv().onclick = () => {this.selectDevice1(device)};
        this.devices.push(device);
    }

    remove () {
        this.devices = this.devices.filter(function(device, index, arr){
            return (device.getId() != this.selectedDevice.getId());
        });
        document.getElementById('deviceDiv' + this.selectedDevice.id).remove();
        this.selectedDevice = null;
    }

    get array () {
        return this.devices;
    }

    getSelectedDevice () {
        return this.selectedDevice;
    }
}

function elementFromHTML (html) {
    const template = document.createElement("template");
    template.innerHTML = html.trim();
    return template.content.firstElementChild;
}