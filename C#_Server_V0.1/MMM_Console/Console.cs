


using MMM_Console;
using MMM_CoreDesktopLibrary;

Console.WriteLine("---------- Mechanical MIDI Music Terminal ----------");
Console.WriteLine("To quit type 'q' or 'quit'.");
Console.WriteLine("For help type 'h' or 'help'.");

while (true)
{
    Console.Write("Mechaical MIDI Music: ");
    string? userInput = Console.ReadLine();
    if (userInput == null || userInput == "") continue;
    string[] cmd = userInput.Split(" ", 2);
    string? pram1 = (cmd.Length > 1) ? cmd[1] : null;
    //string? pram2 = (cmd.Length > 2) ? cmd[2] : null;

    switch (cmd[0].ToUpper())
    {
        case "Q":
        case "QUIT": 
            Cmd_QUIT(); return;
        case "H":
        case "HELP":
            Cmd_HELP(pram1); break;
		case "SA": Cmd_SERIALAVAILABLE(); break;
		case "SERIALAVAILABLE": Cmd_SERIALAVAILABLE(); break;
		case "SL": Cmd_SERIALLIST(); break;
		case "SERIALLIST": Cmd_SERIALLIST(); break;
		case "SC": Cmd_SERIALCONNECT(pram1); break;
		case "SERIALCONNECT": Cmd_SERIALCONNECT(pram1); break;
		case "SD": Cmd_SERIALDISCONNECT(pram1); break;
		case "SERIALDISCONNECT": Cmd_SERIALDISCONNECT(pram1); break;
		case "SR": Cmd_SERIALRESET(); break;
		case "SERIALRESET": Cmd_SERIALRESET(); break;
		case "IA": Cmd_MIDIINAVAILABLE(); break;
		case "MIDIINAVAILABLE": Cmd_MIDIINAVAILABLE(); break;
		case "IL": Cmd_MIDIINLIST(); break;
		case "MIDIINLIST": Cmd_MIDIINLIST(); break;
		case "IC": Cmd_MIDIINCONNECT(pram1); break;
		case "MIDIINCONNECT": Cmd_MIDIINCONNECT(pram1); break;
		case "ID": Cmd_MIDIINDISCONNECT(pram1); break;
		case "MIDIINDISCONNECT": Cmd_MIDIINDISCONNECT(pram1); break;
		case "IR": Cmd_MIDIINRESET(); break;
		case "MIDIINRESET": Cmd_MIDIINRESET(); break;
		case "OA": Cmd_MIDIOUTAVAILABLE(); break;
		case "MIDIOUTAVAILABLE": Cmd_MIDIOUTAVAILABLE(); break;
		case "OL": Cmd_MIDIOUTLIST(); break;
		case "MIDIOUTLIST": Cmd_MIDIOUTLIST(); break;
		case "OC": Cmd_MIDIOUTCONNECT(pram1); break;
		case "MIDIOUTCONNECT": Cmd_MIDIOUTCONNECT(pram1); break;
		case "OD": Cmd_MIDIOUTDISCONNECT(pram1); break;
		case "MIDIOUTDISCONNECT": Cmd_MIDIOUTDISCONNECT(pram1); break;
		case "OR": Cmd_MIDIOUTRESET(); break;
		case "MIDIOUTRESET": Cmd_MIDIOUTRESET(); break;
		case "SONGADD": Cmd_SONGADD(pram1); break;
        case "SONGREMOVE": Cmd_SONGREMOVE(pram1); break;
        case "SONGLIST": Cmd_SONGLIST(); break;
        case "REPEAT": Cmd_REPEAT(pram1); break;
        case "PLAYLISTCLEAR": Cmd_PLAYLISTCLEAR(); break;
        case "LOOP": Cmd_LOOP(pram1); break;
        case "AUTOPLAY": Cmd_AUTOPLAY(pram1); break;
		case "PL": Cmd_PLAY(pram1); break;
		case "PLAY": Cmd_PLAY(pram1); break;
		case "PA": Cmd_PAUSE(); break;
		case "PAUSE": Cmd_PAUSE(); break;
		case "S": Cmd_STOP(); break;
		case "STOP": Cmd_STOP(); break;
        case "START": Cmd_START(); break;
        case "NEXT": Cmd_NEXT(); break;
        case "PREV": Cmd_PREV(); break;

        default:
            Console.WriteLine("Unrecognized Command!");
            Console.WriteLine("For help type 'help'");
            break;
    }
}

void Cmd_QUIT()
{
    MMM_CoreDesktop.Stop();
    Console.WriteLine("MMM Terminated!");
    Console.ReadLine();
}
void Cmd_HELP(string? cmd){
    if (cmd == null) { CommandList.display(); return; }
    CommandList.display(cmd);
}
void Cmd_SERIALAVAILABLE()
{
    var ports = MMM_CoreDesktop.SerialGetAvailable();
    if (ports.Count == 0) { Console.WriteLine("No available Serial ports"); return; }
    Console.WriteLine("---------- Available Serial Ports ----------\n");
    foreach (var port in ports)
    {
        Console.WriteLine("Name: " + port);
    }
    Console.WriteLine();
}
void Cmd_SERIALLIST()
{
    var ports = MMM_CoreDesktop.SerialListConnected();
    if (ports.Count == 0) { Console.WriteLine("No connected Serial ports"); return; }
    Console.WriteLine("---------- Connected Serial Ports ----------\n");
    foreach (var port in ports)
    {
        Console.WriteLine("Name: " + port);
    }
    Console.WriteLine();
}
void Cmd_SERIALCONNECT(string? cmd)
{
    MMM_CoreDesktop.SerialAdd(cmd);
}
void Cmd_SERIALDISCONNECT(string? cmd)
{
    MMM_CoreDesktop.SerialRemove(cmd);
}
void Cmd_SERIALRESET()
{
    Console.WriteLine("TODO");
}
void Cmd_MIDIINAVAILABLE()
{
    var ports = MMM_CoreDesktop.GetAvailableMidiIn();
    if (ports.Count == 0) { Console.WriteLine("No available MIDI In ports");  return; }
    Console.WriteLine("---------- Available MIDI In Ports ----------\n");
    foreach (var port in ports)
    {
        Console.WriteLine("ID: " + port.Item1);
        Console.WriteLine("Name: " + port.Item2);
        Console.WriteLine("Manufacturer: " + port.Item3);
        Console.WriteLine("Version: " + port.Item4);
        Console.WriteLine();
    }
}
void Cmd_MIDIINLIST()
{
    var ports = MMM_CoreDesktop.ListConnectedInputs();
    if (ports.Count == 0) { Console.WriteLine("No connected MIDI In ports"); return; }
    Console.WriteLine("---------- Connected MIDI In Ports ----------\n");
    foreach (var port in ports)
    {
        Console.WriteLine("ID: " + port.Item1);
        Console.WriteLine("Name: " + port.Item2);
        Console.WriteLine("Manufacturer: " + port.Item3);
        Console.WriteLine("Version: " + port.Item4);
        Console.WriteLine();
    }
}
void Cmd_MIDIINCONNECT(string? cmd)
{
    MMM_CoreDesktop.AddMidiInput(cmd);
}
void Cmd_MIDIINDISCONNECT(string? cmd)
{
    MMM_CoreDesktop.RemoveMidiOutput(cmd);
}
void Cmd_MIDIINRESET()
{
    MMM_CoreDesktop.ClearMidiInputs();
}
void Cmd_MIDIOUTAVAILABLE()
{
    var ports = MMM_CoreDesktop.GetAvailableMidiOut();
    if (ports.Count == 0) { Console.WriteLine("No available MIDI Out ports"); return; }
    Console.WriteLine("---------- Available MIDI Out Ports ----------\n");
    foreach (var port in ports)
    {
        Console.WriteLine("ID: " + port.Item1);
        Console.WriteLine("Name: " + port.Item2);
        Console.WriteLine("Manufacturer: " + port.Item3);
        Console.WriteLine("Version: " + port.Item4);
        Console.WriteLine();
    }
}
void Cmd_MIDIOUTLIST()
{
    var ports = MMM_CoreDesktop.ListConnectedOutputs();
    if (ports.Count == 0) { Console.WriteLine("No connected MIDI Out ports"); return; }
    Console.WriteLine("---------- Connected MIDI Out Ports ----------\n");
    foreach (var port in ports)
    {
        Console.WriteLine("ID: " + port.Item1);
        Console.WriteLine("Name: " + port.Item2);
        Console.WriteLine("Manufacturer: " + port.Item3);
        Console.WriteLine("Version: " + port.Item4);
        Console.WriteLine();
    }
}
void Cmd_MIDIOUTCONNECT(string? cmd)
{
    MMM_CoreDesktop.AddMidiOutput(cmd);
}
void Cmd_MIDIOUTDISCONNECT(string? cmd)
{
    MMM_CoreDesktop.RemoveMidiOutput(cmd);
}
void Cmd_MIDIOUTRESET()
{
    MMM_CoreDesktop.ClearMidiOutputs();
}
void Cmd_SONGADD(string? cmd)
{
    try
    {
        MMM_CoreDesktop.AddSongs(cmd);
    }
    catch (Exception e)
    {
        if(e .GetType() == typeof(FileNotFoundException)) { Console.WriteLine("File Not Found!"); return; }
        Console.WriteLine(e);
    }
}
void Cmd_SONGREMOVE(string? cmd)
{
    MMM_CoreDesktop.RemoveSong(cmd);
}
void Cmd_SONGLIST()
{
    var songNames = MMM_CoreDesktop.GetSongNames();
    if (songNames.Count == 0) { Console.WriteLine("No songs in playlist.\n"); return; }
    Console.WriteLine("\n---------- " + songNames.Count + " Songs in Playlist ----------");
    foreach (var name in songNames) {
        Console.WriteLine(name);
    }
    Console.WriteLine();
}
void Cmd_PLAYLISTCLEAR()
{
    MMM_CoreDesktop.PlaylistClear();
}
void Cmd_REPEAT(string? cmd)
{
    MMM_CoreDesktop.RepeatSong(cmd.ToLower() == "true");
}
void Cmd_LOOP(string? cmd)
{
    MMM_CoreDesktop.LoopPlaylist(cmd.ToLower() == "true");
}
void Cmd_AUTOPLAY(string? cmd)
{
    MMM_CoreDesktop.AutoPlay(cmd.ToLower() == "true");
}
void Cmd_PLAY(string? cmd)
{
    if (cmd == null) MMM_CoreDesktop.Play();
    else
    {
        MMM_CoreDesktop.Play(cmd);
        MMM_CoreDesktop.Stop();
        MMM_CoreDesktop.Play();
    }
}
void Cmd_PAUSE()
{
    MMM_CoreDesktop.Pause();
}
void Cmd_STOP()
{
    MMM_CoreDesktop.Stop();
}
void Cmd_START()
{
    MMM_CoreDesktop.Stop();
    MMM_CoreDesktop.Start();
}
void Cmd_NEXT()
{
    MMM_CoreDesktop.IncrementSongIdx();
    MMM_CoreDesktop.Start();
}
void Cmd_PREV()
{
    MMM_CoreDesktop.DecrementSongIdx();
    MMM_CoreDesktop.Start();
}