#pragma once

#include "bdfr/core/Retargeter.h"

#include <string>
#include <vector>

namespace bdfr {

class Arkit52 {
public:
    static const std::vector<std::string>& curveNames();
    static RetargetProfile identityProfile();
};

} // namespace bdfr
