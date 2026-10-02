#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace bdfr {

template <typename T>
class SnapshotHistory {
public:
    explicit SnapshotHistory(std::size_t capacity = 64) : capacity_(capacity == 0 ? 1 : capacity) {}

    void reset(const T& initial) {
        states_.clear();
        states_.push_back(initial);
        index_ = 0;
    }

    void push(const T& state) {
        if (states_.empty()) {
            reset(state);
            return;
        }

        if (index_ + 1 < states_.size())
            states_.erase(states_.begin() + static_cast<std::ptrdiff_t>(index_ + 1), states_.end());

        states_.push_back(state);
        if (states_.size() > capacity_) {
            states_.erase(states_.begin());
        } else {
            ++index_;
        }
        if (index_ >= states_.size()) index_ = states_.size() - 1;
    }

    bool canUndo() const noexcept { return !states_.empty() && index_ > 0; }
    bool canRedo() const noexcept { return !states_.empty() && index_ + 1 < states_.size(); }

    const T* undo() {
        if (!canUndo()) return nullptr;
        --index_;
        return &states_[index_];
    }

    const T* redo() {
        if (!canRedo()) return nullptr;
        ++index_;
        return &states_[index_];
    }

    const T* current() const {
        return states_.empty() ? nullptr : &states_[index_];
    }

    std::size_t size() const noexcept { return states_.size(); }

private:
    std::size_t capacity_;
    std::vector<T> states_;
    std::size_t index_ = 0;
};

} // namespace bdfr
