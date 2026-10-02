#pragma once

#include "bdfr/core/CorrectiveEngine.h"
#include "bdfr/core/CurveMixer.h"

#include <string>
#include <vector>

namespace bdfr {

struct ExpressionLayer {
    std::string name;
    bool enabled = true;
    CurveLayer layer;
};

class ExpressionStack {
public:
    bool addLayer(ExpressionLayer layer);
    bool setEnabled(const std::string& name, bool enabled);
    std::vector<ExpressionLayer>& layers() noexcept;
    const std::vector<ExpressionLayer>& layers() const noexcept;
    CorrectiveEngine& correctives() noexcept;
    const CorrectiveEngine& correctives() const noexcept;
    CurveMap evaluate(const CurveMap& base = {}) const;

private:
    std::vector<ExpressionLayer> layers_;
    CorrectiveEngine correctives_;
};

} // namespace bdfr
