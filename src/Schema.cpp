#include "bdfr/core/Schema.h"

namespace bdfr {

SchemaCompatibility Schema::frameCompatibility(std::uint32_t version) noexcept {
    if (version == CurrentFrameVersion) return SchemaCompatibility::Supported;
    if (version < CurrentFrameVersion) return SchemaCompatibility::TooOld;
    return SchemaCompatibility::TooNew;
}

SchemaCompatibility Schema::projectCompatibility(std::uint32_t version) noexcept {
    if (version == CurrentProjectVersion) return SchemaCompatibility::Supported;
    if (version < CurrentProjectVersion) return SchemaCompatibility::TooOld;
    return SchemaCompatibility::TooNew;
}

bool Schema::normalizeFrame(FacialFrame& frame) noexcept {
    if (frameCompatibility(frame.schemaVersion) != SchemaCompatibility::Supported) return false;
    frame.schemaVersion = CurrentFrameVersion;
    return true;
}

bool Schema::normalizeProject(Project& project) noexcept {
    if (projectCompatibility(project.schemaVersion) != SchemaCompatibility::Supported) return false;
    project.schemaVersion = CurrentProjectVersion;
    return true;
}

} // namespace bdfr
