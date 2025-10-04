/*
 * Minimal NetworkUDP stub
 * Provided so the build can use MultiNetwork<NetworkUDP,...> even if full UDP
 * implementation is not available yet. Implementations are no-ops and should
 * be replaced with the real network code for UDP later.
 */

#pragma once

#include "Network.h"

class NetworkUDP : public Network {
public:
	NetworkUDP() = default;
	void begin() override {
		// no-op stub
	}
	void sendMessage(MidiMessage message) override {
		// no-op stub
	}
	void sendString(String message) override {
		// no-op stub
	}
	std::optional<MidiMessage> readMessage() override {
		return std::nullopt;
	}
};