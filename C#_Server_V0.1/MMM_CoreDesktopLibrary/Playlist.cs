using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;
using System.Threading.Tasks.Sources;

namespace MMM_CoreDesktopLibrary;

internal class Playlist
{
 
    private int curSongIdx = 0;
    private List<Song> playlist = new List<Song>();
    public bool LoopPlaylist { get; set; } = true;

    public void AddSong(Song song) { playlist.Add(song); }
    public void AddSong(int index, Song song) {playlist.Insert(index, song); }

    //Throws File Exceptions.
    public (int, int) AddSongs(String filepath)
    {
        if (File.Exists(filepath))
        {
            AddSong(new Song(filepath));
            return (1,1);
        }
        else if (Directory.Exists(filepath))
        {
            // Print full file paths of all MIDI files in the folder and its subdirectories

            string[] midiFiles = Directory.GetFiles(filepath, "*.mid", SearchOption.AllDirectories);
            int totalSongs = 0;
            int addedSongs = 0;
            foreach (string midiFilePath in midiFiles)
            {
                totalSongs++;
                try
                {
                    AddSong(new Song(midiFilePath));
                    addedSongs++;
                }   
                catch(Exception e) {
                    Debug.WriteLine("");
                    Debug.WriteLine(midiFilePath);
                    Debug.WriteLine(e);
                }
            }
            return (addedSongs, totalSongs);
            


        }
        else
        {
            throw new FileNotFoundException();
        }
    }

    public int GetSongIdx()
    {
        return curSongIdx;
    }
    public int GetSongIdx(string songName)
    {
        return playlist.FindIndex(song => (song.GetName() == songName));
    }
    public void MoveSong(int newIdx, String songName)
    {
        if(newIdx < 0) newIdx = 0;
        if(newIdx > playlist.Count) newIdx = playlist.Count;

        int curIdx = this.GetSongIdx(songName);
        if (curIdx == -1) return;
        Song song = playlist[curIdx];
        this.RemoveSong(songName);

        if (newIdx > curIdx) newIdx--;
        playlist.Insert(newIdx, song);
        curSongIdx = newIdx;

        //if (newIdx > curSongIdx && curSongIdx > curIdx) curSongIdx--;
        //else if (newIdx < curSongIdx && curSongIdx < curIdx) curSongIdx++;
    }

    //Catch Out of Bounds Exception
    //Returns True if song is removed.
    public bool RemoveSong(int index)
    {
        if (playlist.Remove(playlist[index])){
            // Decrement cur song index as to not skip the next song.
            if (curSongIdx > index) curSongIdx--;
            return true;
        }
        return false;
    }

    //Returns True if song is removed.
    public bool RemoveSong(String songName)
    {
        Song? foundSong = playlist.Find(song => song.GetName().Equals(songName, StringComparison.OrdinalIgnoreCase));
        if(foundSong == null) return false; 
        int idx = this.GetSongIdx(songName);
        if (playlist.Remove((Song)foundSong)){
            // Decrent cur song index as to not skip the next song.
            if(curSongIdx > idx) curSongIdx--;
            return true;
        }
        return false;
    }

    public void Clear()
    {
        playlist.Clear();
    }

    public List<Song> GetSongs()
    {
        return playlist;
    }

    //Returns True if song idx is set.
    public bool SetCurSong(String songName)
    {
        int idx = GetSongIdx(songName);
        if (idx < 0) return false;
        curSongIdx = idx;
        return true;
    }

    //Returns True if song idx is set.
    public bool SetCurSong(int idx)
    {
        if (idx < 0 || idx >= playlist.Count) return false;
        curSongIdx = idx;
        return true;
    }

    public Song? GetCurSong()
    {
        if(playlist.Count == 0) return null;
        return playlist[curSongIdx];
    }

    public int Count()
    {
        return playlist.Count();
    }

    public Song? IncrementSongIdx()
    {
        if (playlist.Count == 0) throw new IndexOutOfRangeException();
        curSongIdx++;
        if (curSongIdx >= playlist.Count)
        {
            curSongIdx = 0;
            if (!LoopPlaylist) return null;
        }
        return playlist[curSongIdx];
    }

    public Song DecrementSongIdx()
    {
        if (playlist.Count == 0 ) throw new IndexOutOfRangeException();
        curSongIdx--;
        if (curSongIdx < 0) curSongIdx = playlist.Count-1;
        return playlist[curSongIdx];
    }
}

