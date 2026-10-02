#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <vector>

namespace bdfr {

class FacialSequence {
public:
    bool addFrame(FacialFrame frame);
    const std::vector<FacialFrame>& frames() const noexcept;
    double durationSeconds() const noexcept;
    bool validate(std::string* error = nullptr) const;
    FacialFrame sample(double timeSeconds) const;

private:
    std::vector<FacialFrame> frames_;
};

} // namespace bdfr
