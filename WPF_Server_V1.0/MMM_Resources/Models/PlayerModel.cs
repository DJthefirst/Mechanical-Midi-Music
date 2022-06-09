using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MMM_Resources.Models
{
    public class PlayerModel
    {
        public int BPM { get; set; }
        public int Volume { get; set; }
        public double CurrentPosition { get; set; }
        public double CurrentDuration { get; set; }
        public List<String> Songs { get; set; }
        public List<String> MidiIn { get; set; }
        public List<String> MidiOut { get; set; }
        public bool IsPlaying { get; set; }
        public bool IsRepeat { get; set; }
        public bool IsMuted { get; set; }
        public bool IsOverideMidi { get; set; }


    }
}