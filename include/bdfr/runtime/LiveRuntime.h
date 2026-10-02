#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstddef>
#include <deque>
#include <mutex>

namespace bdfr::runtime {

class FrameQueue {
public:
    explicit FrameQueue(std::size_t capacity);

    void push(FacialFrame frame);
    bool pop(FacialFrame& frame);
    std::size_t size() const;
    std::size_t capacity() const noexcept;
    std::size_t droppedCount() const;

private:
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::deque<FacialFrame> frames_;
    std::size_t dropped_ = 0;
};

class JitterBuffer {
public:
    explicit JitterBuffer(double delaySeconds = 0.05);

    void setDelay(double delaySeconds);
    double delay() const noexcept;
    void push(FacialFrame frame);
    bool popReady(double localPlaybackSeconds, FacialFrame& frame);
    std::size_t size() const noexcept;

private:
    double delaySeconds_;
    std::deque<FacialFrame> frames_;
};

class ClockOffsetEstimator {
public:
    explicit ClockOffsetEstimator(double alpha = 0.1);

    void reset();
    void observe(double remoteTimestampSeconds, double localArrivalSeconds);
    bool initialized() const noexcept;
    double offsetSeconds() const noexcept;
    double remoteToLocal(double remoteTimestampSeconds) const noexcept;

private:
    double alpha_;
    double offset_ = 0.0;
    bool initialized_ = false;
};

} // namespace bdfr::runtime
