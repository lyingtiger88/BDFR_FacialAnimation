#include "bdfr/core/SequenceCleanup.h"
#include "bdfr/core/SequenceDiagnostics.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

bool near(float a, float b, float epsilon = 0.0001F) {
    return std::fabs(a - b) <= epsilon;
}

} // namespace

int main() {
    bdfr::FacialSequence sequence;

    bdfr::FacialFrame a;
    a.timestampSeconds = 0.0;
    a.confidence = 0.95F;
    a.curves["jawOpen"] = 0.20F;
    a.curves["AU12"] = 0.30F;

    bdfr::FacialFrame b;
    b.timestampSeconds = 0.1;
    b.confidence = 0.40F;
    b.curves["jawOpen"] = 0.95F;
    b.curves["AU12"] = 0.32F;

    bdfr::FacialFrame c;
    c.timestampSeconds = 0.2;
    c.confidence = 0.90F;
    c.curves["jawOpen"] = 0.22F;
    c.curves["AU12"] = 0.31F;

    sequence.addFrame(a);
    sequence.addFrame(b);
    sequence.addFrame(c);

    const auto report =
        bdfr::SequenceDiagnostics::analyze(
            sequence, 0.5F, 0.4F, 0.05F);

    expect(report.lowConfidenceFrames == 1,
           "diagnostics finds low-confidence frame");
    expect(report.spikes.size() == 1,
           "diagnostics finds isolated jaw spike");
    expect(report.spikes.front().curveId == "jawOpen",
           "diagnostics identifies spiking curve");
    expect(report.meanConfidence > 0.70F,
           "diagnostics computes mean confidence");

    const auto cleaned =
        bdfr::SequenceCleanup::despike(
            sequence, 0.4F, 0.05F);

    expect(cleaned.frames().size() == 3,
           "cleanup preserves frame count");
    expect(near(cleaned.frames()[1].curves.at("jawOpen"), 0.21F, 0.011F),
           "cleanup replaces isolated spike with neighbor expectation");
    expect(near(cleaned.frames()[1].curves.at("AU12"), 0.32F),
           "cleanup preserves non-spiking curve");

    const auto cleanedReport =
        bdfr::SequenceDiagnostics::analyze(
            cleaned, 0.5F, 0.4F, 0.05F);

    expect(cleanedReport.spikes.empty(),
           "cleanup removes detected spike");

    if (failures == 0) {
        std::cout << "All BDFR quality tests passed.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
