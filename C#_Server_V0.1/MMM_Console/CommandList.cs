
using System;
using System.Reflection;

namespace MMM_Console;

internal struct Command
{
    public Command(string name, string description, string documentation)
    {
        this.Name = name;
        this.Description = description;
        this.Documentation = documentation;
    }
    public string Name { get; set; }
    public string Description { get; set; }
    public string Documentation { get; set; }
}

internal static class CommandList
{
    static List<Command> commands;
    static CommandList()
    {
        commands = new List<Command>();
        addCommands();
    }

    public static void display()
    {
        Console.WriteLine("\n------------------------------ HELP ------------------------------");
        Console.WriteLine("For more information on a specific command, type HELP command-name");
        foreach (var command in commands)
        {
            Console.WriteLine(command.Name.PadRight(20, ' ') 
                + command.Description);
        }
        Console.WriteLine();
    }

    public static void display(string cmdName)
    {
        int index = commands.FindIndex((cmd => cmd.Name.ToUpper() == cmdName.ToUpper()));
        if (index == -1) {
            Console.WriteLine("Invalid Command: '" + cmdName + "'");
            Console.WriteLine("For help type 'h' or 'help'.");
            return;
        }
        Console.WriteLine("\n---------- " + commands[index].Name + " ----------");
        Console.WriteLine("\nDescription:\n" + commands[index].Description + "\n");
        Console.WriteLine(commands[index].Documentation);
    }

    private static void addCommands()
    {
        commands.Add(new Command(
            "QUIT",
            "Teminates the program.",
            "Parameters: None \n" +
            "Ex. Usage: QUIT \n" 
            ));
        commands.Add(new Command(
            "HELP",
            "Displays Command Use.",
            "Parameters: None \n" +
            "Ex. Usage: HELP \n\n" +
            "Parameters: CommandName \n" +
            "Ex. Usage: HELP quit\n"
            ));
        commands.Add(new Command(
            "SERIALAVAILABLE",
            "Lists available serial ports for connection.",
            "Parameters: None \n" +
            "Ex. Usage: SERIALAVAILABLE \n"
            ));
        commands.Add(new Command(
            "SERIALLIST",
            "Lists serial ports connected to serial MIDI output.",
            "Parameters: None \n" +
            "Ex. Usage: SERIALLIST \n"
            ));
        commands.Add(new Command(
            "SERIALCONNECT",
            "Connects a serial port to serial MIDI output.",
            "Parameters: SerialPortID \n" +
            "Ex. Usage: SERIALCONNECT COM5 \n"
            ));
        commands.Add(new Command(
            "SERIALDISCONNECT",
            "Disconnects a serial port from serial MIDI output.",
            "Parameters: SerialPortID \n" +
            "Ex. Usage: SERIALDISCONNECT COM5 \n"
            ));
        commands.Add(new Command(
            "SERIALRESET",
            "Disconnects all serial ports from serial MIDI output.",
            "Parameters: None \n" +
            "Ex. Usage: SERIALRESET \n"
            ));


        commands.Add(new Command(
            "MIDIINAVAILABLE",
            "Lists available MIDI In ports for connection.",
            "Parameters: None \n" +
            "Ex. Usage: MIDIINAVAILABLE \n"
            ));
        commands.Add(new Command(
            "MIDIINLIST",
            "Lists MIDI In ports connected to MIDI Input.",
            "Parameters: None \n" +
            "Ex. Usage: MIDIINLIST \n"
            ));
        commands.Add(new Command(
            "MIDIINCONNECT",
            "Connects a MIDI In port to MIDI Input.",
            "Parameters: MidiInPortID \n" +
            "Ex. Usage: MIDIINCONNECT LoopMidi0 \n"
            ));
        commands.Add(new Command(
            "MIDIINDISCONNECT",
            "Disconnects a MIDI In port from MIDI Input.",
            "Parameters: MidiInPortID \n" +
            "Ex. Usage: MIDIINDISCONNECT LoopMidi0 \n"
            ));
        commands.Add(new Command(
            "MIDIINRESET",
            "Disconnects all Midi In ports from Midi Input.",
            "Parameters: None \n" +
            "Ex. Usage: MIDIINRESET \n"
            ));


        commands.Add(new Command(
            "MIDIOUTAVAILABLE",
            "Lists available MIDI Out ports for connection.",
            "Parameters: None \n" +
            "Ex. Usage: MIDIOUTAVAILABLE \n"
            ));
        commands.Add(new Command(
            "MIDIOUTLIST",
            "Lists MIDI Out ports connected to MIDI Output.",
            "Parameters: None \n" +
            "Ex. Usage: MIDIOUTLIST \n"
            ));
        commands.Add(new Command(
            "MIDIOUTCONNECT",
            "Connects a MIDI Out port to MIDI Output.",
            "Parameters: MidiOutPortID \n" +
            "Ex. Usage: MIDIOUTCONNECT LoopMidi0 \n"
            ));
        commands.Add(new Command(
            "MIDIOUTDISCONNECT",
            "Disconnects a MIDI Out port from MIDI Output.",
            "Parameters: MidiOutPortID \n" +
            "Ex. Usage: MIDIOUTDISCONNECT LoopMidi0 \n"
            ));
        commands.Add(new Command(
            "MIDIOUTRESET",
            "Disconnects all Midi Out ports from Midi Output.",
            "Parameters: None \n" +
            "Ex. Usage: MIDIOUTRESET \n"
            ));


        commands.Add(new Command(
            "SONGADD",
            "Add a song or folder(non recursive) to the playlist.",
            "Parameters: FilePath \n" +
            "Ex. Usage: SONGADD C:\\Users\\user1234\\Documents\\Tetris.mid\n\n" +
            "Parameters: DirectoryPath \n" +
            "Ex. Usage: SONGADD C:\\Users\\user1234\\Documents\\Songs\n"
            ));
        commands.Add(new Command(
            "SONGREMOVE",
            "Remove the current or specified song from the playlist",
            "Parameters: SongName \n" +
            "Ex. Usage: SONGREMOVE Tetris \n\n" +
            "Parameters: SongName \n" +
            "Ex. Usage: SONGREMOVE Tetris \n"
            ));
        commands.Add(new Command(
            "SONGLIST",
            "List all songs in the current playlist.",
            "Parameters: None \n" +
            "Ex. SONGLIST \n"
            ));
        commands.Add(new Command(
            "PLAYLISTCLEAR",
            "Clear all songs in the playlist.",
            "Parameters: None \n" +
            "Ex. Usage: SONGSCLEAR \n"
            ));
        commands.Add(new Command(
            "REPEAT",
            "Enables the current song to repeat.",
            "Default: False \n" +
            "Parameters: True/False \n" +
            "Ex. Usage: SONGREPEAT False \n"
            ));
        commands.Add(new Command(
            "LOOP",
            "Enables lopping of last song to first song in playlist.",
            "Default: True \n" +
            "Parameters: True/False \n" +
            "Ex. Usage: LOOP True \n"
            ));
        commands.Add(new Command(
            "AUTOPLAY",
            "Enables automantic playing of the next song in the playlist.",
            "Default: True \n" +
            "Parameters: True/False \n" +
            "Ex. Usage: AUTOPLAY True \n"
            ));


        commands.Add(new Command(
            "PLAY",
            "Plays the current or specified song.",
            "Parameters: None \n" +
            "Ex. Usage: PLAY \n\n" +
            "Parameters: SongName \n" +
            "Ex. Usage: PLAY Tetris\n"
            ));
        commands.Add(new Command(
            "PAUSE",
            "Pauses the current song.",
            "Parameters: None \n" +
            "Ex. Usage: PAUSE \n"
            ));
        commands.Add(new Command(
            "STOP",
            "Stops the current song.",
            "Parameters: None \n" +
            "Ex. Usage: STOP \n"
            ));
        commands.Add(new Command(
            "START",
            "Plays the current song from the start.",
            "Parameters: None \n" +
            "Ex. Usage: START \n"
            ));
        commands.Add(new Command(
            "NEXT",
            "Sets the current song to the next in the playlist",
            "Parameters: None \n" +
            "Ex. Usage: NEXT \n"
            ));
        commands.Add(new Command(
            "PREV",
            "Sets the current song to the previous in the playlist",
            "Parameters: None \n" +
            "Ex. Usage: PREV \n"
            ));
    }
}

