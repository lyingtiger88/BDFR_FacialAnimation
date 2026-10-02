#include "bdfr/capture/OfflineVideoProcessor.h"

namespace bdfr::capture {

OfflineSolveResult OfflineVideoProcessor::process(
    IVideoSource& source,
    IFaceTracker& tracker,
    IFaceSolver& solver,
    const OfflineSolveOptions& options) {

    OfflineSolveResult result;
    source.reset();
    tracker.reset();
    solver.reset();

    VideoFrame videoFrame;
    while (source.next(videoFrame)) {
        ++result.sourceFrames;

        OfflineFrameReport report;
        report.timestampSeconds = videoFrame.timestampSeconds;

        const ImageView image = videoFrame.view();
        FaceObservation observation;

        if (!image.valid() || !tracker.process(image, observation)) {
            ++result.trackerFailures;
            result.frames.push_back(report);
            if (options.stopOnTrackerFailure) break;
            continue;
        }

        report.trackerSucceeded = true;
        report.quality = CaptureQuality::evaluate(
            observation, options.minimumQuality);

        if (options.skipLowQualityFrames && report.quality.badTake) {
            ++result.skippedFrames;
            result.frames.push_back(report);
            continue;
        }

        FacialFrame solved;
        if (!solver.solve(observation, solved)) {
            ++result.solverFailures;
            result.frames.push_back(report);
            if (options.stopOnSolverFailure) break;
            continue;
        }

        report.solverSucceeded = true;
        solved.timestampSeconds = videoFrame.timestampSeconds;
        solved.confidence = clamp01(
            solved.confidence * report.quality.overall);

        if (result.sequence.addFrame(std::move(solved))) {
            ++result.solvedFrames;
            report.included = true;
        }

        result.frames.push_back(report);
    }

    return result;
}

} // namespace bdfr::capture
