#include "bdfr/core/SequenceCompare.h"

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

bdfr::FacialSequence makeSequence(float middleJaw) {
    bdfr::FacialSequence sequence;

    bdfr::FacialFrame a;
    a.timestampSeconds = 0.0;
    a.curves["jawOpen"] = 0.0F;
    a.curves["AU12"] = 0.2F;

    bdfr::FacialFrame b;
    b.timestampSeconds = 0.5;
    b.curves["jawOpen"] = middleJaw;
    b.curves["AU12"] = 0.4F;

    bdfr::FacialFrame c;
    c.timestampSeconds = 1.0;
    c.curves["jawOpen"] = 0.0F;
    c.curves["AU12"] = 0.2F;

    sequence.addFrame(a);
    sequence.addFrame(b);
    sequence.addFrame(c);
    return sequence;
}

} // namespace

int main() {
    const auto reference = makeSequence(0.8F);
    const auto identical = makeSequence(0.8F);
    const auto changed = makeSequence(0.4F);

    const auto sameReport =
        bdfr::SequenceCompare::compare(
            reference,
            identical,
            60.0);

    expect(sameReport.sampleCount > 0,
           "comparison samples timeline");
    expect(sameReport.meanAbsoluteError < 0.000001,
           "identical sequences have zero mean error");
    expect(sameReport.maximumAbsoluteError < 0.000001F,
           "identical sequences have zero maximum error");

    const auto changedReport =
        bdfr::SequenceCompare::compare(
            reference,
            changed,
            60.0);

    expect(changedReport.meanAbsoluteError > 0.0,
           "changed sequence produces nonzero mean error");
    expect(changedReport.maximumAbsoluteError > 0.35F,
           "changed sequence exposes strong peak difference");

    const auto it =
        changedReport.curves.find("jawOpen");

    expect(it != changedReport.curves.end(),
           "comparison reports changed curve");
    expect(it != changedReport.curves.end() &&
           it->second.maximumAbsoluteError > 0.35F,
           "per-curve report preserves peak jaw error");

    expect(changedReport.curves.at("AU12").maximumAbsoluteError <
               0.000001F,
           "unchanged curve stays at zero error");

    if (failures == 0) {
        std::cout << "All BDFR sequence comparison tests passed.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
