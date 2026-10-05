#include "bdfr/core/SequencePlayback.h"

#include <algorithm>
#include <cmath>

namespace bdfr {

void SequencePlayback::load(const FacialSequence* sequence) noexcept {
    sequence_ = sequence;
    positionSeconds_ = 0.0;
    state_ = PlaybackState::Stopped;
}

void SequencePlayback::play() noexcept {
    if (!sequence_ || sequence_->frames().empty()) return;
    if (positionSeconds_ >= durationSeconds() && !looping_)
        positionSeconds_ = 0.0;
    state_ = PlaybackState::Playing;
}

void SequencePlayback::pause() noexcept {
    if (state_ == PlaybackState::Playing)
        state_ = PlaybackState::Paused;
}

void SequencePlayback::stop() noexcept {
    state_ = PlaybackState::Stopped;
    positionSeconds_ = 0.0;
}

void SequencePlayback::seek(double seconds) noexcept {
    if (!std::isfinite(seconds)) return;
    positionSeconds_ = std::clamp(seconds, 0.0, durationSeconds());
}

double SequencePlayback::durationSeconds() const noexcept {
    return sequence_ ? sequence_->durationSeconds() : 0.0;
}

FacialFrame SequencePlayback::update(double deltaSeconds) {
    if (!sequence_ || sequence_->frames().empty())
        return {};

    if (state_ == PlaybackState::Playing &&
        std::isfinite(deltaSeconds) &&
        deltaSeconds > 0.0) {
        positionSeconds_ += deltaSeconds;
        const double duration = durationSeconds();

        if (duration > 0.0 && positionSeconds_ > duration) {
            if (looping_) {
                positionSeconds_ = std::fmod(positionSeconds_, duration);
            } else {
                positionSeconds_ = duration;
                state_ = PlaybackState::Stopped;
            }
        }
    }

    return sequence_->sample(positionSeconds_);
}

} // namespace bdfr
