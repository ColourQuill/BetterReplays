#include <buffer.hpp>

// better replays
#include <logger.hpp>

void Buffer::addPacket(const EncodedPacket& packet) {
    std::lock_guard<std::mutex> lock(mutex);
    packets.push_back(packet);
    trim();
}

void Buffer::trim() {
    if (packets.size() < 2) return;

    int64_t currentDuration = packets.back().presentationTimestamp - packets.front().presentationTimestamp;

    while (currentDuration > targetDurationMS && packets.size() > 1) {
        packets.pop_front();

        while (!packets.empty() && !packets.front().isKeyframe) {
            packets.pop_front();
        }

        if (packets.size() < 2) break;

        currentDuration = packets.back().presentationTimestamp - packets.front().presentationTimestamp;
    }
}

std::vector<EncodedPacket> Buffer::getSnapshot() {
    std::lock_guard<std::mutex> lock(mutex);

    std::vector<EncodedPacket> snapshot(packets.begin(), packets.end());
    if (snapshot.empty()) return snapshot;

    int64_t base = snapshot.front().presentationTimestamp;

    Logger::logInfo("Buffer", "Snapshot base PTS: " + std::to_string(base) 
        + " first DTS: " + std::to_string(snapshot.front().decodeTimestamp));

    for (auto& packet : snapshot) {
        packet.presentationTimestamp -= base;
        packet.decodeTimestamp       -= base;
    }

    return snapshot;
}