using Commons.Music.Midi;
using CoreMidi;

namespace MMM_CoreDesktopLibrary;

public struct Song
{

    private string name = "";
    private string filepath;
    public Song(string filepath)
    {
        //Validate filePath
        if (!File.Exists(filepath)) throw new FileNotFoundException();
        if (!filepath.EndsWith("mid", false, null)) throw new InvalidFileTypeException(filepath, "*.mid");
        MidiMusic.Read(System.IO.File.OpenRead(filepath));

        this.filepath = filepath;
        this.name = Path.GetFileName(filepath).Split(".", 2)[0]; //File name removing the file type.
    }
    
    public string GetName() { return name; }
    public string GetPath() { return filepath; }
}

//Exception designated for invalid file type.
public class InvalidFileTypeException : System.IO.IOException
{
    public InvalidFileTypeException(string path, string acceptedTypeMask)
        : base(string.Format("File type '{0}' does not fall within the expected range: '{1}'",
            path,
            acceptedTypeMask)){}
}