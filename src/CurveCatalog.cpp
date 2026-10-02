#include "bdfr/core/CurveCatalog.h"
#include "bdfr/core/CurveRegistry.h"

#include <algorithm>

namespace bdfr {

namespace {

std::string counterpartFor(const std::string& id) {
    const std::string left = "Left";
    const std::string right = "Right";
    if (id.size() >= left.size() && id.compare(id.size() - left.size(), left.size(), left) == 0)
        return id.substr(0, id.size() - left.size()) + right;
    if (id.size() >= right.size() && id.compare(id.size() - right.size(), right.size(), right) == 0)
        return id.substr(0, id.size() - right.size()) + left;
    return {};
}

} // namespace

CurveCatalog::CurveCatalog() {
    CurveRegistry registry;
    for (const auto& id : registry.canonicalCurves()) {
        CurveMetadata meta;
        meta.id = id;
        meta.region = CurveMixer::classify(id);
        meta.facsActionUnit = id.rfind("AU", 0) == 0;
        meta.counterpart = counterpartFor(id);
        meta.asymmetric = !meta.counterpart.empty();
        metadata_.emplace(id, std::move(meta));
    }
}

const CurveMetadata* CurveCatalog::metadata(const std::string& curveId) const {
    auto it = metadata_.find(curveId);
    return it == metadata_.end() ? nullptr : &it->second;
}

std::vector<CurveMetadata> CurveCatalog::all() const {
    std::vector<CurveMetadata> out;
    out.reserve(metadata_.size());
    for (const auto& [_, meta] : metadata_) out.push_back(meta);
    std::sort(out.begin(), out.end(),
        [](const CurveMetadata& a, const CurveMetadata& b) { return a.id < b.id; });
    return out;
}

} // namespace bdfr
