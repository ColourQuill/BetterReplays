#include <buffer.hpp>

void Buffer::addPacket(const EncodedPacket& packet) {
    std::lock_guard<std::mutex> lock(mutex);
    packets.push_back(packet);
    trim();
}

void Buffer::trim() {
    if (packets.size() < 2) {
        return;
    }

    int64_t currentDuration = packets.back().presentationTimestamp - packets.front().presentationTimestamp;

    while (currentDuration > targetDurationMS && packets.size() > 1) {
        packets.pop_front();

        while (!packets.empty() && !packets.front().isKeyframe) {
            packets.pop_front();
        }

        if (packets.empty()) {
            currentDuration = packets.back().presentationTimestamp - packets.front().presentationTimestamp;
        } else {
            break;
        }
    }
}

std::vector<EncodedPacket> Buffer::getSnapshot() {
    std::lock_guard<std::mutex> lock(mutex);
    return {packets.begin(), packets.end()};
}