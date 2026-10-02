#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstdint>
#include <vector>

namespace bdfr {

class BinaryCodec {
public:
    static std::vector<std::uint8_t> encode(const FacialFrame& frame);
    static bool decode(const std::vector<std::uint8_t>& bytes, FacialFrame& outFrame);
};

} // namespace bdfr
