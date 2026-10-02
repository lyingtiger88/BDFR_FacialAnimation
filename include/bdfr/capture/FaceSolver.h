#pragma once

#include "bdfr/capture/FaceObservation.h"

namespace bdfr::capture {

class IFaceSolver {
public:
    virtual ~IFaceSolver() = default;

    virtual bool solve(const FaceObservation& observation,
                       FacialFrame& frame) = 0;

    virtual void reset() = 0;
};

} // namespace bdfr::capture
