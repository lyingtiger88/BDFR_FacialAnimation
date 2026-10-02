#include "bdfr/core/ExpressionStack.h"

#include <algorithm>
#include <utility>

namespace bdfr {

bool ExpressionStack::addLayer(ExpressionLayer layer) {
    if (layer.name.empty()) return false;
    const auto duplicate = std::find_if(layers_.begin(), layers_.end(),
        [&](const ExpressionLayer& existing) { return existing.name == layer.name; });
    if (duplicate != layers_.end()) return false;
    layers_.push_back(std::move(layer));
    return true;
}

bool ExpressionStack::setEnabled(const std::string& name, bool enabled) {
    auto it = std::find_if(layers_.begin(), layers_.end(),
        [&](const ExpressionLayer& layer) { return layer.name == name; });
    if (it == layers_.end()) return false;
    it->enabled = enabled;
    return true;
}

std::vector<ExpressionLayer>& ExpressionStack::layers() noexcept {
    return layers_;
}

const std::vector<ExpressionLayer>& ExpressionStack::layers() const noexcept {
    return layers_;
}

CorrectiveEngine& ExpressionStack::correctives() noexcept {
    return correctives_;
}

const CorrectiveEngine& ExpressionStack::correctives() const noexcept {
    return correctives_;
}

CurveMap ExpressionStack::evaluate(const CurveMap& base) const {
    std::vector<CurveLayer> active;
    for (const auto& expressionLayer : layers_) {
        if (expressionLayer.enabled) active.push_back(expressionLayer.layer);
    }
    return correctives_.apply(CurveMixer::mix(base, std::move(active)));
}

} // namespace bdfr
