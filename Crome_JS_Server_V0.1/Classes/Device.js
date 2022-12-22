var currentDevId = 0;

export default class Device {
    constructor(port, ComType) {
      this.port = port;
      this.comType = ComType;
      this.instrumentType = 'Instrument';
      this.id = currentDevId;
      this.div = this.createDiv();
      currentDevId++;
    }

    // Generate HTML for Device //
    createDiv () {
        let deviceDiv = document.createElement('div');
        deviceDiv.id = 'deviceDiv' + this.id;
        deviceDiv.className = 'deviceDiv';
        deviceDiv.appendChild(elementFromHTML(' \
            <div class="deviceInfo"> \
                <b class="deviceInfo">Dev: '+ this.id +'</b>\
                <b class="deviceInfo"> ' + this.instrumentType + '</b>\
                <b class="deviceInfo">' + this.comType +'</b>\
            </div> \
            <div> \
            </div> \
        '));
        return deviceDiv
    }

    getId() { return this.id; }
    getPort() { return this.port; }
    getDiv() { return this.div; }
}

function elementFromHTML (html) {
    const template = document.createElement("template");
    template.innerHTML = html.trim();
    return template.content.firstElementChild;
}