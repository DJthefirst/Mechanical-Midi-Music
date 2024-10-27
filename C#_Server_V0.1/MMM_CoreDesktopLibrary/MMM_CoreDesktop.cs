using Commons.Music.Midi;
using CoreMidi;
using System.IO.Ports;
using System.Runtime.InteropServices;

namespace MMM_CoreDesktopLibrary;


public static class MMM_CoreDesktop
{
    private static MidiCore midiCore;
    //private static ReMapper reMapper = new ReMapper();
    private static MidiIO midiIO;
    private static SerialIO serialIO;
    private static DeviceConfig deviceConfig;
    private static Playlist playlist;
    private static MMM_MidiPlayer player;

    static MMM_CoreDesktop()
    {
        midiCore = new MidiCore();
        //ReMapper reMapper = new ReMapper();
        midiIO = new MidiIO(midiCore);
        serialIO = new SerialIO();
        DeviceConfig deviceConfig = new DeviceConfig();
        playlist = new Playlist();
        player = new MMM_MidiPlayer(ref playlist, ref midiCore);
    }


    //------------------------------PlayList------------------------------//


    public static void AddSong(Song song) { }
    public static void AddSong(int index, Song song) { playlist.AddSong(index, song); }

    //Throws File Exceptions.
    public static (int, int) AddSongs(String filepath)
    {
        return playlist.AddSongs(filepath);
    }

    public static int GetSongIdx()
    {
        return playlist.GetSongIdx();
    }

    public static List<Song> GetSongs()
    {
        return playlist.GetSongs();
    }

    public static List<String> GetSongNames()
    {
        return playlist.GetSongs().ConvertAll(song => song.GetName());
    }

    public static int GetSongIdx(string songName)
    {
        return playlist.GetSongIdx(songName);
    }
    public static void MoveSong(int newIdx, String songName)
    {
        playlist.MoveSong(newIdx, songName);
    }

    //Catch Out of Bounds Exception
    //Returns True if song is removed.
    public static bool RemoveSong(int index)
    {
        return playlist.RemoveSong(index);
    }

    //Returns True if song is removed.
    public static bool RemoveSong(String songName)
    {
        return playlist.RemoveSong(songName);
    }

    public static void PlaylistClear()
    {
        playlist.Clear();
    }

    //Returns True if song idx is set.
    public static bool SetCurSong(String songName)
    {
        return playlist.SetCurSong(songName);
    }

    //Returns True if song idx is set.
    public static bool SetCurSong(int idx)
    {
        return playlist.SetCurSong(idx);
    }

    public static Song? GetCurSong()
    {
        return playlist.GetCurSong();
    }

    public static int PlaylistSize()
    {
        return playlist.Count();
    }

    public static void IncrementSongIdx()
    {
        playlist.IncrementSongIdx();
        player.SetSong();
        player.Stop();
        if (player.GetPlaying()) player.Play();
    }

    public static void DecrementSongIdx()
    {
        playlist.DecrementSongIdx();
        player.SetSong();
        player.Stop();
        if (player.GetPlaying()) player.Play();
    }


    //------------------------------Player------------------------------//

    public static void PlayerUpdateSong()
    {
        player.SetSong();
    }

    public static void RepeatSong(bool doRepeat)
    {
        player.RepeatSong = doRepeat;
    }

    public static void LoopPlaylist(bool doLoop)
    {
        playlist.LoopPlaylist = doLoop;
    }

    public static void AutoPlay(bool doAutoPlay)
    {
        player.AutoPlay = doAutoPlay;
    }

    //Return True if song is playing.
    public static bool Play(Song song)
    {
        return player.Play(song);
    }

    public static bool Play(String songName)
    {
        if (!playlist.SetCurSong(songName))return false;
        player.SetSong();
        return true;
    }

    //Return True if song is playing.
    public static bool Play()
    {
        return player.Play();
    }

    public static void Play(MidiMusic song)
    {
        player.Play(song);
        player.Stop();
        player.Play();
    }

    public static void Pause() { player.Pause(); }
    public static void Stop() { player.Stop(); }
    public static void Start() { player.Start(); }
    public static bool GetPlaying() { return player.GetPlaying(); }


    //----------------------------MidiIO------------------------------

    public static List<(string, string, string, string)> GetAvailableMidiIn()
    {
        return midiIO.GetAvailableMidiIn();
    }

    public static List<(string, string, string, string)> GetAvailableMidiOut()
    {
        return midiIO.GetAvailableMidiOut();
    }

    public static List<(string, string, string, string)> ListConnectedInputs()
    {
        return midiIO.ListConnectedInputs();
    }

    public static List<(string, string, string, string)> ListConnectedOutputs()
    {
        return midiIO.ListConnectedOutputs();
    }

    public static void AddMidiInput(string Id)
    {
        midiIO.AddInput(Id);
    }

    public static void AddMidiOutput(string Id)
    {
        midiIO.AddOutput(Id);
    }

    public static bool RemoveMidiInput(string Id)
    {
        return midiIO.RemoveInput(Id);
    }

    public static bool RemoveMidiOutput(string Id)
    {
        return midiIO.RemoveOutput(Id);
    }

    public static void ClearMidiInputs()
    {
        midiIO.ClearInputs();
    }

    public static void ClearMidiOutputs()
    {
        midiIO.ClearOutputs();
    }

    //------------------------------Serial Ports------------------------------

    public static List<String> SerialGetAvailable()
    {
        return serialIO.GetAvailable();
    }
    public static List<String> SerialListConnected()
    {
        return serialIO.ListConnected();
    }
    public static void SerialAdd(string portName)
    {
        serialIO.Add(portName);
    }

    public static void SerialRemove(string portName)
    {
        //Add a Stop command.
        serialIO.Remove(portName);
    }

    public static void SerialClear()
    {
        serialIO.Clear();
    }
}