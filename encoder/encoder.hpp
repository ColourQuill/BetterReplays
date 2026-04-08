#ifndef __ENCODER_HPP__
#define __ENCODER_HPP__

// better replays
#include <frame.hpp>
#include <encoded_packet.hpp>
#include <settings.hpp>

// std
#include <functional>
#include <cstdint>
#include <vector>

using PacketCallback = std::function<void(const EncodedPacket&)>;

class Encoder {
    public:
        virtual ~Encoder() = default;

        virtual bool init(int srcWidth, int srcHeight, int dstWidth, int dstHeight, int fps) = 0;
        virtual bool encode(const Frame& frame) = 0;
        virtual void flush() = 0;

        void setPacketCallback(PacketCallback packetCallback) { onPacket = std::move(packetCallback); }
        void setSettings(Settings* settings) { this->settings = settings; }

        static Encoder* create();
    protected:
        PacketCallback onPacket;

        Settings* settings = nullptr;
};

#endif