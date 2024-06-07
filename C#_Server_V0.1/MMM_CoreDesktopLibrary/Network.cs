using Commons.Music.Midi;
using CoreMidi;

namespace MMM_CoreDesktopLibrary;

internal interface INetwork
{
    public List<String> GetAvailable();
    public List<String> ListConnected();
    public void Add(string Id);
    public void Remove(string Id);
    public void Clear();
}

internal interface INetworkIn : INetwork
{

}

internal interface INetworkOut : INetwork
{

}