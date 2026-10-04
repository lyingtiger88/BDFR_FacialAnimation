#include "bdfr/metahuman/MetaHumanRig.h"

#include <utility>

namespace bdfr::metahuman {

struct MetaHumanRigRuntime::Impl {
    MetaHumanRigInfo info;
};

MetaHumanRigRuntime::MetaHumanRigRuntime()
    : impl_(std::make_unique<Impl>()) {}

MetaHumanRigRuntime::~MetaHumanRigRuntime() = default;

MetaHumanRigRuntime::MetaHumanRigRuntime(MetaHumanRigRuntime&&) noexcept = default;
MetaHumanRigRuntime& MetaHumanRigRuntime::operator=(MetaHumanRigRuntime&&) noexcept = default;

bool MetaHumanRigRuntime::backendAvailable() noexcept {
    return false;
}

const char* MetaHumanRigRuntime::backendName() noexcept {
    return "Unavailable";
}

bool MetaHumanRigRuntime::loadDna(const std::string&, std::string* error) {
    if (error) {
        *error = "BDFR was built without OpenRigLogic support.";
    }
    return false;
}

void MetaHumanRigRuntime::unload() {
    impl_->info = {};
}

bool MetaHumanRigRuntime::isLoaded() const noexcept {
    return false;
}

const MetaHumanRigInfo& MetaHumanRigRuntime::info() const noexcept {
    return impl_->info;
}

std::vector<std::string> MetaHumanRigRuntime::rawControlNames() const {
    return {};
}

std::vector<std::string> MetaHumanRigRuntime::meshNames() const {
    return {};
}

bool MetaHumanRigRuntime::extractMesh(
    std::uint16_t,
    MetaHumanMeshData&,
    std::string* error) const {
    if (error) {
        *error = "OpenRigLogic backend is unavailable.";
    }
    return false;
}

bool MetaHumanRigRuntime::evaluate(
    const CurveMap&,
    MetaHumanRigOutput&,
    std::string* error) {
    if (error) {
        *error = "OpenRigLogic backend is unavailable.";
    }
    return false;
}

bool MetaHumanRigRuntime::evaluate(
    const FacialFrame& frame,
    MetaHumanRigOutput& output,
    std::string* error) {
    return evaluate(frame.curves, output, error);
}

std::vector<std::pair<std::string, std::uint16_t>>
MetaHumanRigRuntime::resolveMappings(const CurveMap&) const {
    return {};
}

} // namespace bdfr::metahuman
