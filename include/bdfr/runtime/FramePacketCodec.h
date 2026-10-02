#pragma once

#include "bdfr/mocap/ExternalMocap.h"

#include <cstdint>
#include <vector>

namespace bdfr::runtime {

class FramePacketCodec {
public:
    static std::vector<std::uint8_t> encode(const mocap::MocapPacket& packet);
    static bool decode(const std::vector<std::uint8_t>& bytes,
                       mocap::MocapPacket& packet);
};

} // namespace bdfr::runtime
