#pragma once

#include "bdfr/core/FacialTypes.h"
#include "bdfr/core/Project.h"

#include <cstdint>

namespace bdfr {

enum class SchemaCompatibility {
    Supported,
    TooOld,
    TooNew
};

class Schema {
public:
    static constexpr std::uint32_t CurrentFrameVersion = 1;
    static constexpr std::uint32_t CurrentProjectVersion = 1;

    static SchemaCompatibility frameCompatibility(std::uint32_t version) noexcept;
    static SchemaCompatibility projectCompatibility(std::uint32_t version) noexcept;

    static bool normalizeFrame(FacialFrame& frame) noexcept;
    static bool normalizeProject(Project& project) noexcept;
};

} // namespace bdfr
