#pragma once

#include "bdfr/core/Sequence.h"

#include <string>

namespace bdfr::mocap {

struct MocapCsvOptions {
    std::string timestampColumn = "time";
    char delimiter = ',';
    bool clampCurves = true;
};

class MocapCsv {
public:
    static bool decode(
        const std::string& csv,
        FacialSequence& sequence,
        const MocapCsvOptions& options = {},
        std::string* error = nullptr);

    static std::string encode(
        const FacialSequence& sequence,
        char delimiter = ',');
};

} // namespace bdfr::mocap
