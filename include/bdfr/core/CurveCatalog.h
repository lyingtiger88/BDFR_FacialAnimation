#pragma once

#include "bdfr/core/CurveMixer.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace bdfr {

struct CurveMetadata {
    std::string id;
    CurveRegion region = CurveRegion::Mouth;
    bool facsActionUnit = false;
    bool asymmetric = false;
    std::string counterpart;
};

class CurveCatalog {
public:
    CurveCatalog();

    const CurveMetadata* metadata(const std::string& curveId) const;
    std::vector<CurveMetadata> all() const;

private:
    std::unordered_map<std::string, CurveMetadata> metadata_;
};

} // namespace bdfr
