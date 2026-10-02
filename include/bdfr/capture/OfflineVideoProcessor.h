#pragma once

#include "bdfr/capture/CaptureQuality.h"
#include "bdfr/capture/FaceSolver.h"
#include "bdfr/capture/VideoSource.h"
#include "bdfr/core/Sequence.h"

#include <cstddef>
#include <vector>

namespace bdfr::capture {

struct OfflineSolveOptions {
    float minimumQuality = 0.35F;
    bool skipLowQualityFrames = false;
    bool stopOnTrackerFailure = false;
    bool stopOnSolverFailure = false;
};

struct OfflineFrameReport {
    double timestampSeconds = 0.0;
    CaptureQualityReport quality;
    bool trackerSucceeded = false;
    bool solverSucceeded = false;
    bool included = false;
};

struct OfflineSolveResult {
    FacialSequence sequence;
    std::vector<OfflineFrameReport> frames;
    std::size_t sourceFrames = 0;
    std::size_t solvedFrames = 0;
    std::size_t skippedFrames = 0;
    std::size_t trackerFailures = 0;
    std::size_t solverFailures = 0;
};

class OfflineVideoProcessor {
public:
    static OfflineSolveResult process(IVideoSource& source,
                                      IFaceTracker& tracker,
                                      IFaceSolver& solver,
                                      const OfflineSolveOptions& options = {});
};

} // namespace bdfr::capture
