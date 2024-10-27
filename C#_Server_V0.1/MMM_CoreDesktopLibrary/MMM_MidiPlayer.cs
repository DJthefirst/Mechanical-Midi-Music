using Commons.Music.Midi;
using System.Diagnostics;

namespace MMM_CoreDesktopLibrary;

internal class MMM_MidiPlayer
{
    private MidiCore midiOut;
    private Playlist playlist;
    private Song? curSong = null;
    private MidiMusic? curSongData = null;
    private MidiPlayer? midiPlayer = null;
    public bool RepeatSong { get; set; } = false;
    public bool AutoPlay { get; set; } = true;
    private bool isPlaying = false;


    public MMM_MidiPlayer(ref Playlist playlist, ref MidiCore midiOut)
    {
        this.playlist = playlist;
        this.midiOut = midiOut;
    }

    //Throws File Exception if file changed
    public void SetSong()
    {
        curSong = playlist.GetCurSong();
        if (curSong == null)
        {
            curSongData = null;
            return;
        }
        curSongData = MidiMusic.Read(System.IO.File.OpenRead(((Song)curSong).GetPath()));
    }

    //Return True if song is playing.
    public bool Play(Song song)
    {
        if (playlist.SetCurSong(song.GetName())) return false;
        return Play();
    }

    //Return True if song is playing.
    public bool Play()
    {
        if (midiPlayer != null) { midiPlayer.Play(); isPlaying = true; return true; }
        if (playlist.Count() == 0) return false;
        if (!AutoPlay) { Stop(); return false; }
        try
        {
            SetSong();
            if (curSongData == null) return false;
            Play(curSongData);
            return true;
        }
        catch (Exception e)
        {
            Debug.WriteLine(e);
            return false;
        }
    }

    public void Play(MidiMusic song)
    {
        midiPlayer = new MidiPlayer(song, midiOut);
        midiPlayer.PlaybackCompletedToEnd += () => {
            isPlaying = false;
            if(!RepeatSong) playlist.IncrementSongIdx();
            SetSong();
            if(curSongData == null) { midiPlayer = null; return; }
            Play(curSongData);
        };
        midiPlayer.Play();
        isPlaying = true;
    }

    public void Pause() { if (midiPlayer != null) midiPlayer.Pause(); isPlaying = false; }
    public void Stop() { 
        if (midiPlayer != null) 
        {
            midiPlayer.Stop(); 
            midiPlayer.Dispose(); 
        } 
        midiPlayer = null; 
        isPlaying = false; 
     }
    public void Start() { if (midiPlayer != null) midiPlayer.Stop(); this.Play(); }

    public bool GetPlaying() { return isPlaying; }


}

