using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;



using MMM_Networking;
using MMM_MIDI_Controller;


namespace MMM_Controller
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
    public partial class MainWindow : Window
    {
        TextBoxOutputter outputter;

        UDPServer server = new UDPServer();
        MidiController controller = new MidiController();

        string display = "default";

        public MainWindow()
        {
            InitializeComponent();

            outputter = new TextBoxOutputter(TestBox);
            Console.SetOut(outputter);
            Console.WriteLine("Started");

            server.UDP_PacketReceivedEvent += UDPServer_PacketReceivedEvent;
        }

        private void textChangedEventHandler(object sender, TextChangedEventArgs args)
        {
            TestBox.ScrollToEnd();
        }

            private void UDPServer_PacketReceivedEvent(object? sender, string packet_data)
        {
            display = packet_data;
            //TextBlockLog.Text = display;
        }

        private void HandleEvent1(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("display");
        }

        private void HandleInitilize(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Initilize Server");
            server.Initialize();
            server.StartListener();
        }

        private void HandleStartMIDI(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Initalize Midi");
            controller.Initialize(server);
        }

        private void HandlePlayMIDI(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Start Midi");
            controller.Play();
        }

        private void HandleStopMIDI(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Stop Midi");
            controller.Stop();
        }

        private void HandlePlayNoteMIDI(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Play Midi");
            controller.PlayNotes();
        }

        private void HandleTestMIDI(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Test");

        }

        private void HandlePing(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("Ping");
            server.Ping();
        }

        private void HandleUDPmsg(object sender, RoutedEventArgs e)
        {
            Console.WriteLine("msg");
            byte[] data = { 0x00, 0x11, 0x22, 0x33, 0x44 };
            server.SendToMulticast(data);
        }

        //private void HandleDropDown(object sender, RoutedEventArgs e)
        //{
        //    var deviceList = DeviceInformation.FindAllAsync(
        // MidiInPort.GetDeviceSelector());
        //
        //    foreach (var deviceInfo in deviceList)
        //    {
        //        Console.WriteLine(deviceInfo.Id);
        //        Console.WriteLine(deviceInfo.Name);
        //        Console.WriteLine("----------");
        //    }
        //    Console.WriteLine("finish - no args");
        //
        //}
    }
}

