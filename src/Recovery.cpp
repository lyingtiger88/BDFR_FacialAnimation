#include "bdfr/core/Recovery.h"
#include "bdfr/core/Persistence.h"

#include <algorithm>
#include <system_error>

namespace bdfr {

namespace {

std::filesystem::path snapshotPath(const std::filesystem::path& directory,
                                   const std::string& baseName,
                                   std::size_t index) {
    return directory / (baseName + ".recovery." + std::to_string(index) + ".bdfr.json");
}

} // namespace

bool Recovery::saveRotatingSnapshot(const std::filesystem::path& directory,
                                    const std::string& baseName,
                                    const Project& project,
                                    std::size_t maxSnapshots,
                                    std::string* error) {
    if (baseName.empty() || maxSnapshots == 0) {
        if (error) *error = "invalid recovery configuration";
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec) {
        if (error) *error = "failed to create recovery directory: " + ec.message();
        return false;
    }

    for (std::size_t i = maxSnapshots; i-- > 1;) {
        const auto from = snapshotPath(directory, baseName, i - 1);
        const auto to = snapshotPath(directory, baseName, i);
        if (!std::filesystem::exists(from)) continue;

        std::filesystem::remove(to, ec);
        ec.clear();
        std::filesystem::rename(from, to, ec);
        if (ec) {
            if (error) *error = "failed rotating recovery snapshot: " + ec.message();
            return false;
        }
    }

    return Persistence::saveProject(snapshotPath(directory, baseName, 0), project, error);
}

std::filesystem::path Recovery::latestSnapshot(const std::filesystem::path& directory,
                                               const std::string& baseName) {
    const auto path = snapshotPath(directory, baseName, 0);
    return std::filesystem::exists(path) ? path : std::filesystem::path{};
}

} // namespace bdfr
