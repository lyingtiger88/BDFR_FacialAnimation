#pragma once

#include "bdfr/mocap/ExternalMocap.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace bdfr::runtime {

class SessionStream {
public:
    static std::vector<std::uint8_t> encode(
        const std::vector<mocap::MocapPacket>& packets);

    static bool decode(
        const std::vector<std::uint8_t>& bytes,
        std::vector<mocap::MocapPacket>& packets,
        std::string* error = nullptr);

    static bool save(
        const std::filesystem::path& path,
        const std::vector<mocap::MocapPacket>& packets,
        std::string* error = nullptr);

    static bool load(
        const std::filesystem::path& path,
        std::vector<mocap::MocapPacket>& packets,
        std::string* error = nullptr);
};

} // namespace bdfr::runtime
