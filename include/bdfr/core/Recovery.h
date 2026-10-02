#pragma once

#include "bdfr/core/Project.h"

#include <cstddef>
#include <filesystem>
#include <string>

namespace bdfr {

class Recovery {
public:
    static bool saveRotatingSnapshot(const std::filesystem::path& directory,
                                     const std::string& baseName,
                                     const Project& project,
                                     std::size_t maxSnapshots = 5,
                                     std::string* error = nullptr);

    static std::filesystem::path latestSnapshot(const std::filesystem::path& directory,
                                                const std::string& baseName);
};

} // namespace bdfr
