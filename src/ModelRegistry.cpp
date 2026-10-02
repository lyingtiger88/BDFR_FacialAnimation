#include "bdfr/models/ModelRegistry.h"

#include <algorithm>
#include <utility>

namespace bdfr::models {

namespace {

std::string keyFor(
    const std::string& id,
    const std::string& version) {

    return id + "@" + version;
}

} // namespace

bool ModelRegistry::registerModel(
    ModelManifest model) {

    if (model.id.empty() ||
        model.version.empty() ||
        model.fileName.empty() ||
        model.license.empty()) {
        return false;
    }

    const std::string key =
        keyFor(model.id, model.version);

    return models_.emplace(
        key,
        std::move(model)).second;
}

const ModelManifest*
ModelRegistry::find(
    const std::string& id,
    const std::string& version) const {

    const auto it =
        models_.find(keyFor(id, version));

    return it == models_.end()
        ? nullptr
        : &it->second;
}

const ModelManifest*
ModelRegistry::find(
    const std::string& id) const {

    const ModelManifest* best = nullptr;

    for (const auto& [_, model] : models_) {
        if (model.id != id) continue;

        if (!best ||
            model.version > best->version) {
            best = &model;
        }
    }

    return best;
}

std::vector<ModelManifest>
ModelRegistry::all() const {

    std::vector<ModelManifest> out;
    out.reserve(models_.size());

    for (const auto& [_, model] : models_) {
        out.push_back(model);
    }

    std::sort(
        out.begin(),
        out.end(),
        [](const ModelManifest& a,
           const ModelManifest& b) {
            if (a.id != b.id) {
                return a.id < b.id;
            }
            return a.version < b.version;
        });

    return out;
}

} // namespace bdfr::models
