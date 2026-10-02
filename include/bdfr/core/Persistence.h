#pragma once

#include "bdfr/core/Project.h"

#include <filesystem>
#include <string>

namespace bdfr {

class Persistence {
public:
    static bool saveTextAtomic(const std::filesystem::path& path,
                               const std::string& content,
                               std::string* error = nullptr);

    static bool loadText(const std::filesystem::path& path,
                         std::string& content,
                         std::string* error = nullptr);

    static bool saveProject(const std::filesystem::path& path,
                            const Project& project,
                            std::string* error = nullptr);

    static bool loadProject(const std::filesystem::path& path,
                            Project& project,
                            std::string* error = nullptr);
};

} // namespace bdfr
