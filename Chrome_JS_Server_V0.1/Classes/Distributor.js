export default class Distributor {
    constructor(id) {
      this.id = id;
      this.channels;
      this.instruments;
      this.distributionMethod;

      this.boolean;
      this.boolDamperEnable;
      this.boolPolyphonicEnable;
      this.NoteOveriteEnaable;

      this.minNote;
      this.maxNote;
      this.numPolyphonicNotes;
    }

    setDistributorConstruct(bytes){
        this.id = bytes[0] << 7 & bytes[1];
        this.channels = (bytes[2] << 7) & bytes[3];
        this.instruments = (bytes[4] << 24) & (bytes[5] << 16) & (bytes[6] << 8) & bytes[7];
        this.distributionMethod = bytes[8];
        this.boolean = bytes[9];
        this.minNote = bytes[11];
        this.maxNote = bytes[12];
        this.numPolyphonicNotes = bytes[13];
    }

    // Generate HTML for Device //
    createDiv () {
        let distributorDiv = document.createElement('div');
        distributorDiv.id = 'distributorDiv' + this.id;
        distributorDiv.className = 'distributorDiv';
        distributorDiv.appendChild(elementFromHTML(' \
            <div class="deviceInfo"> \
                <b class="deviceInfo">Dev: '+ this.id +'</b>\
                <b class="deviceInfo"> ' + this.instrumentType + '</b>\
                <b class="deviceInfo">' + this.comType +'</b>\
                <br>\
                <select class=DistributorDropDown id="DistributorDropDownDevice'+ this.id +'">\
                    <option value="" selected="selected"> Distributor 0 </option>\
                </select>\
            </div> \
            <div> \
            </div> \
        '));
        return distributorDiv
    }
}