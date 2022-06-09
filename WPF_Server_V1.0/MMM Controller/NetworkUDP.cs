using System;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Text;
using System.Windows;

namespace MMM_Networking
{

    public class UDPServer
    {
        public event EventHandler<string> UDP_PacketReceivedEvent;

        public const int MULTICASTPORT = 65534;
        public const int UNICASTPORT = 65535;

        private Socket _multicast_socket;
        private Socket _unicast_socket;
        private EndPoint _multicast_ep;
        private EndPoint _unicast_ep;

        private IPAddress _multicast_ip = IPAddress.Parse("224.5.6.7");
        private IPAddress[] _unicast_clients;

        private byte[] _buffer_recv;
        private ArraySegment<byte> _buffer_recv_segment;

        //Initilize Multicase Send and Unicast Listener
        public void Initialize()
        {
            //Create a buffer for incoming messages
            _buffer_recv = new byte[4096];
            _buffer_recv_segment = new(_buffer_recv);

            //Set multicast IP and unicast client IP 
            _multicast_ep = new IPEndPoint(_multicast_ip, MULTICASTPORT);
            _unicast_ep = new IPEndPoint(IPAddress.Any, UNICASTPORT);

            //Settup Sockets
            _multicast_socket = new(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            _unicast_socket = new(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

            //Set Multitask TTL / Scope
            _multicast_socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.MulticastTimeToLive, 8);

            //Return Info on Unicast Connections
            _unicast_socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.PacketInformation, true);

            _multicast_socket.Connect(_multicast_ep);
            _unicast_socket.Bind(_unicast_ep);
        }

        //Start Listener
        public void StartListener()
        {
            _ = Task.Run(async () =>
            {
                SocketReceiveMessageFromResult res;
                while (true)
                {
                    res = await _unicast_socket.ReceiveMessageFromAsync(_buffer_recv_segment, SocketFlags.None, _unicast_ep);
                    //string received = Encoding.UTF8.GetString(_buffer_recv, 0, res.ReceivedBytes);
                    string received = BitConverter.ToString(_buffer_recv, 0, res.ReceivedBytes).Replace("-", " ");
                    Console.WriteLine(received);
                    UDP_PacketReceivedEvent?.Invoke(this, received);
                }
            });
        }

        //Ping Clients
        public async Task Ping()
        {
            await SendToMulticast(Encoding.UTF8.GetBytes("Ping"));
        }

        //Send Multicast Packet
        public async Task SendToMulticast(byte[] data)
        {
            var buffer = new ArraySegment<byte>(data);
            await _multicast_socket.SendToAsync(buffer, SocketFlags.None, _multicast_ep);

        }
    }

    public class UDPClient
    {
        private Socket _socket;
        private EndPoint _ep;

        private byte[] _buffer_recv;
        private ArraySegment<byte> _buffer_recv_segment;

        public void Initialize(IPAddress address, int port)
        {
            _buffer_recv = new byte[4096];
            _buffer_recv_segment = new(_buffer_recv);

            _ep = new IPEndPoint(address, port);

            _socket = new(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            _socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.PacketInformation, true);
        }

        public void StartMessageLoop()
        {
            _ = Task.Run(async () =>
            {
                SocketReceiveMessageFromResult res;
                while (true)
                {
                    res = await _socket.ReceiveMessageFromAsync(_buffer_recv_segment, SocketFlags.None, _ep);
                }
            });
        }

        public async Task Send(byte[] data)
        {
            var s = new ArraySegment<byte>(data);
            await _socket.SendToAsync(s, SocketFlags.None, _ep);
        }
    }

    public class TestPrint
    {
        public void Print()
        {
            Console.WriteLine("Hello World!");
        }
    }
}