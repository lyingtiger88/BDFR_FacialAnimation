#pragma once

#include "bdfr/core/Sequence.h"

namespace bdfr {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

class SequencePlayback {
public:
    void load(const FacialSequence* sequence) noexcept;
    void play() noexcept;
    void pause() noexcept;
    void stop() noexcept;
    void seek(double seconds) noexcept;
    void setLooping(bool enabled) noexcept { looping_ = enabled; }
    bool looping() const noexcept { return looping_; }

    FacialFrame update(double deltaSeconds);

    PlaybackState state() const noexcept { return state_; }
    double positionSeconds() const noexcept { return positionSeconds_; }
    double durationSeconds() const noexcept;

private:
    const FacialSequence* sequence_{nullptr};
    PlaybackState state_{PlaybackState::Stopped};
    double positionSeconds_{0.0};
    bool looping_{false};
};

} // namespace bdfr
