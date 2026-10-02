#include "bdfr/core/SequenceCompare.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace bdfr {

SequenceComparisonReport SequenceCompare::compare(
    const FacialSequence& a,
    const FacialSequence& b,
    double sampleRateHz) {

    SequenceComparisonReport report;

    if (!std::isfinite(sampleRateHz) || sampleRateHz <= 0.0) {
        return report;
    }

    const double duration =
        std::max(a.durationSeconds(), b.durationSeconds());

    report.durationSeconds = duration;
    report.sampleRateHz = sampleRateHz;

    if (duration <= 0.0) {
        return report;
    }

    const double step = 1.0 / sampleRateHz;
    double totalError = 0.0;
    std::size_t totalSamples = 0;

    for (double time = 0.0;
         time <= duration + step * 0.25;
         time += step) {

        const FacialFrame fa = a.sample(time);
        const FacialFrame fb = b.sample(time);

        std::unordered_set<std::string> ids;
        for (const auto& [id, _] : fa.curves) ids.insert(id);
        for (const auto& [id, _] : fb.curves) ids.insert(id);

        for (const auto& id : ids) {
            const auto ia = fa.curves.find(id);
            const auto ib = fb.curves.find(id);

            const float av =
                ia == fa.curves.end() ? 0.0F : ia->second;

            const float bv =
                ib == fb.curves.end() ? 0.0F : ib->second;

            const float error =
                std::fabs(av - bv);

            CurveComparison& curve =
                report.curves[id];

            curve.meanAbsoluteError += error;
            curve.maximumAbsoluteError =
                std::max(
                    curve.maximumAbsoluteError,
                    error);
            ++curve.samples;

            totalError += error;
            report.maximumAbsoluteError =
                std::max(
                    report.maximumAbsoluteError,
                    error);
            ++totalSamples;
        }

        ++report.sampleCount;
    }

    for (auto& [_, curve] : report.curves) {
        if (curve.samples > 0) {
            curve.meanAbsoluteError /=
                static_cast<double>(curve.samples);
        }
    }

    report.meanAbsoluteError =
        totalSamples == 0
            ? 0.0
            : totalError /
              static_cast<double>(totalSamples);

    return report;
}

} // namespace bdfr
