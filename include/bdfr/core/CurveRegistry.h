#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace bdfr {

class CurveRegistry {
public:
    CurveRegistry();

    bool isRegistered(const std::string& id) const;
    bool registerCustom(const std::string& id);
    const std::vector<std::string>& canonicalCurves() const noexcept;

    bool validateFrame(const FacialFrame& frame, std::string* error = nullptr) const;
    FacialFrame sanitized(const FacialFrame& frame) const;

private:
    std::vector<std::string> canonical_;
    std::unordered_set<std::string> known_;
};

} // namespace bdfr
