#include "bdfr/runtime/LiveRuntime.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bdfr::runtime {

FrameQueue::FrameQueue(std::size_t capacity)
    : capacity_(std::max<std::size_t>(1, capacity)) {}

void FrameQueue::push(FacialFrame frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (frames_.size() >= capacity_) {
        frames_.pop_front();
        ++dropped_;
    }
    frames_.push_back(std::move(frame));
}

bool FrameQueue::pop(FacialFrame& frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (frames_.empty()) return false;
    frame = std::move(frames_.front());
    frames_.pop_front();
    return true;
}

std::size_t FrameQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_.size();
}

std::size_t FrameQueue::capacity() const noexcept {
    return capacity_;
}

std::size_t FrameQueue::droppedCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return dropped_;
}

JitterBuffer::JitterBuffer(double delaySeconds)
    : delaySeconds_(std::max(0.0, delaySeconds)) {}

void JitterBuffer::setDelay(double delaySeconds) {
    if (std::isfinite(delaySeconds)) delaySeconds_ = std::max(0.0, delaySeconds);
}

double JitterBuffer::delay() const noexcept {
    return delaySeconds_;
}

void JitterBuffer::push(FacialFrame frame) {
    auto it = std::upper_bound(frames_.begin(), frames_.end(), frame.timestampSeconds,
        [](double time, const FacialFrame& value) { return time < value.timestampSeconds; });
    frames_.insert(it, std::move(frame));
}

bool JitterBuffer::popReady(double localPlaybackSeconds, FacialFrame& frame) {
    if (frames_.empty() || !std::isfinite(localPlaybackSeconds)) return false;
    const double cutoff = localPlaybackSeconds - delaySeconds_;
    if (frames_.front().timestampSeconds > cutoff) return false;
    frame = std::move(frames_.front());
    frames_.pop_front();
    return true;
}

std::size_t JitterBuffer::size() const noexcept {
    return frames_.size();
}

ClockOffsetEstimator::ClockOffsetEstimator(double alpha)
    : alpha_(std::clamp(alpha, 0.001, 1.0)) {}

void ClockOffsetEstimator::reset() {
    offset_ = 0.0;
    initialized_ = false;
}

void ClockOffsetEstimator::observe(double remoteTimestampSeconds, double localArrivalSeconds) {
    if (!std::isfinite(remoteTimestampSeconds) || !std::isfinite(localArrivalSeconds)) return;
    const double sample = localArrivalSeconds - remoteTimestampSeconds;
    if (!initialized_) {
        offset_ = sample;
        initialized_ = true;
        return;
    }
    offset_ += (sample - offset_) * alpha_;
}

bool ClockOffsetEstimator::initialized() const noexcept {
    return initialized_;
}

double ClockOffsetEstimator::offsetSeconds() const noexcept {
    return offset_;
}

double ClockOffsetEstimator::remoteToLocal(double remoteTimestampSeconds) const noexcept {
    return remoteTimestampSeconds + (initialized_ ? offset_ : 0.0);
}

} // namespace bdfr::runtime
