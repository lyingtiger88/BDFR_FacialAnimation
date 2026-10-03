#include "bdfr/runtime/LiveSessionReceiver.h"

#include <algorithm>

namespace bdfr::runtime {

LiveSessionReceiver::LiveSessionReceiver(double jitterDelaySeconds)
    : jitter_(jitterDelaySeconds),
      clock_(0.1) {}

bool LiveSessionReceiver::open(std::uint16_t port,
                               const std::string& bindAddress,
                               std::string* error) {
    close();
    return receiver_.open(port, bindAddress, error);
}

void LiveSessionReceiver::close() {
    receiver_.close();
    jitter_ = JitterBuffer(jitter_.delay());
    clock_.reset();
    stats_ = {};
}

bool LiveSessionReceiver::isOpen() const noexcept {
    return receiver_.isOpen();
}

std::uint16_t LiveSessionReceiver::localPort() const noexcept {
    return receiver_.localPort();
}

bool LiveSessionReceiver::poll(int timeoutMilliseconds,
                               double localArrivalSeconds,
                               std::string* error) {
    mocap::MocapPacket packet;

    std::string localError;
    std::string* receiveError = error ? error : &localError;
    receiveError->clear();

    if (!receiver_.receive(packet, timeoutMilliseconds, receiveError)) {
        // A normal select/recv timeout is expected while waiting for live
        // traffic and must not be reported as a transport/decode failure.
        if (!receiveError->empty()) {
            ++stats_.decodeOrReceiveFailures;
        }
        return false;
    }

    ++stats_.packetsReceived;
    stats_.lastSourceId = packet.sourceId;

    if (stats_.hasSequence) {
        if (packet.sequenceNumber > stats_.lastSequenceNumber + 1) {
            stats_.packetsLost +=
                packet.sequenceNumber - stats_.lastSequenceNumber - 1;
        } else if (packet.sequenceNumber <= stats_.lastSequenceNumber) {
            ++stats_.outOfOrderPackets;
        }
    }

    if (!stats_.hasSequence ||
        packet.sequenceNumber > stats_.lastSequenceNumber) {
        stats_.lastSequenceNumber = packet.sequenceNumber;
        stats_.hasSequence = true;
    }

    clock_.observe(packet.frame.timestampSeconds, localArrivalSeconds);

    FacialFrame localFrame = packet.frame;
    localFrame.timestampSeconds =
        clock_.remoteToLocal(packet.frame.timestampSeconds);

    jitter_.push(std::move(localFrame));

    stats_.clockOffsetSeconds = clock_.offsetSeconds();
    stats_.bufferedFrames = jitter_.size();
    return true;
}

bool LiveSessionReceiver::popReady(double localPlaybackSeconds,
                                   FacialFrame& frame) {
    const bool ready = jitter_.popReady(localPlaybackSeconds, frame);
    stats_.bufferedFrames = jitter_.size();
    return ready;
}

const LiveReceiverStats& LiveSessionReceiver::stats() const noexcept {
    return stats_;
}

} // namespace bdfr::runtime
