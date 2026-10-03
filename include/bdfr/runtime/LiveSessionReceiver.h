#pragma once

#include "bdfr/runtime/LiveRuntime.h"
#include "bdfr/runtime/UdpTransport.h"

#include <cstdint>
#include <string>

namespace bdfr::runtime {

struct LiveReceiverStats {
    std::uint64_t packetsReceived = 0;
    std::uint64_t packetsLost = 0;
    std::uint64_t outOfOrderPackets = 0;
    std::uint64_t decodeOrReceiveFailures = 0;
    std::uint64_t lastSequenceNumber = 0;
    bool hasSequence = false;
    std::string lastSourceId;
    double clockOffsetSeconds = 0.0;
    std::size_t bufferedFrames = 0;
};

class LiveSessionReceiver {
public:
    explicit LiveSessionReceiver(double jitterDelaySeconds = 0.05);

    bool open(std::uint16_t port,
              const std::string& bindAddress = "0.0.0.0",
              std::string* error = nullptr);

    void close();
    bool isOpen() const noexcept;
    std::uint16_t localPort() const noexcept;

    bool poll(int timeoutMilliseconds,
              double localArrivalSeconds,
              std::string* error = nullptr);

    bool popReady(double localPlaybackSeconds,
                  FacialFrame& frame);

    const LiveReceiverStats& stats() const noexcept;

private:
    UdpFrameReceiver receiver_;
    JitterBuffer jitter_;
    ClockOffsetEstimator clock_;
    LiveReceiverStats stats_;
};

} // namespace bdfr::runtime
