#ifndef __BUFFER_HPP__
#define __BUFFER_HPP__

// std
#include <deque>
#include <mutex>

// better replays
#include <encoded_packet.hpp>

class Buffer {
    public:
        Buffer(int64_t durationMS) : targetDurationMS{durationMS} {} 

        void addPacket(const EncodedPacket& packet);
        std::vector<EncodedPacket> getSnapshot();
    private:
        std::deque<EncodedPacket> packets;
        std::mutex mutex;
        int64_t targetDurationMS;

        void trim();
};

#endif