#pragma once
// Only include network headers if their macros are defined
#ifdef MMM_NETWORK_SERIAL
#include "Networks/NetworkSerial.h"
#endif
#ifdef MMM_NETWORK_USB
#include "Networks/NetworkUSB.h"
#endif
#ifdef MMM_NETWORK_UDP
#include "Networks/NetworkUDP.h"
#endif
#ifdef MMM_NETWORK_DIN
#include "Networks/NetworkDIN.h"
#endif

#include "INetwork.h"
#include <vector>
#include <memory>

// Dynamic MultiNetwork backed by a vector. The factory can push_back compiled-in
// networks without needing combinatorial preprocessor defines.
class NetworkManager : public INetwork {
private:
    std::vector<std::unique_ptr<INetwork>> m_networks;

public:
    NetworkManager() = default;

    template<typename NetT, typename... Args>
    void addNetwork(Args&&... args) {
        m_networks.push_back(std::make_unique<NetT>(std::forward<Args>(args)...));
    }

    void begin() override {
        for (auto& net : m_networks) if (net) net->begin();
    }
    size_t numberOfNetworks() const {
        return m_networks.size();
    }

    void sendMessage(MidiMessage message) override {
        for (auto& net : m_networks) if (net) net->sendMessage(message);
    }

    void sendString(String message) override {
        for (auto& net : m_networks) if (net) net->sendString(message);
    }

    std::optional<MidiMessage> readMessage() override {
        for (auto& net : m_networks) {
            if (!net) continue;
            auto msg = net->readMessage();
            if (msg.has_value()) return msg;
        }
        return std::nullopt;
    }
};

// Factory: create a MultiNetwork and push compiled-in network instances into it.
inline std::unique_ptr<NetworkManager> CreateNetwork(){
    auto net = std::make_unique<NetworkManager>();

#ifdef MMM_NETWORK_SERIAL
    net->addNetwork<NetworkSerial>();
#endif
#ifdef MMM_NETWORK_USB
    net->addNetwork<NetworkUSB>();
#endif
#ifdef MMM_NETWORK_UDP
    net->addNetwork<NetworkUDP>();
#endif
#ifdef MMM_NETWORK_DIN
    net->addNetwork<NetworkDIN>();
#endif
    if(net->numberOfNetworks() == 0){
        // No networks compiled in - handle error as appropriate
        // For now, we just return an empty MultiNetwork
    }

    return net;
}