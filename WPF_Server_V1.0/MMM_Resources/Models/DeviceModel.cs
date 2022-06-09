using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MMM_Resources.Models
{
    public class DeviceModel
    {
        //Name of Device
        public string DeviceName { get; set; }
        //Network ID
        public string DeviceID { get; set; }
        //Enum Type of Instrument
        //public Instrument DeviceType { get; set; }
        //Connection Handler
        //public NetCon Connection { get; set; }


        //Highest Playable Note
        public int NoteMin { get; set; }
        //Lowest Playable Note
        public int NoteMax { get; set; }
        //Supported Attributes
        public bool PitchBend { get; set; }
        public bool AfterTouch { get; set; }


        //public Distributor[] DeviceDistributors{ get; set; }
    }
}
