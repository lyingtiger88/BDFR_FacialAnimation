#include "bdfr/mocap/MocapCsv.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace bdfr::mocap {

namespace {

std::string trim(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() &&
           std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }

    std::size_t end = value.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(begin, end - begin);
}

std::vector<std::string> split(
    const std::string& line,
    char delimiter) {

    std::vector<std::string> fields;
    std::string current;
    bool quoted = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];

        if (c == '"') {
            if (quoted &&
                i + 1 < line.size() &&
                line[i + 1] == '"') {
                current.push_back('"');
                ++i;
            } else {
                quoted = !quoted;
            }
            continue;
        }

        if (c == delimiter && !quoted) {
            fields.push_back(trim(current));
            current.clear();
            continue;
        }

        current.push_back(c);
    }

    fields.push_back(trim(current));
    return fields;
}

bool parseDouble(
    const std::string& text,
    double& value) {

    try {
        std::size_t consumed = 0;
        value = std::stod(text, &consumed);
        return consumed == text.size();
    } catch (...) {
        return false;
    }
}

} // namespace

bool MocapCsv::decode(
    const std::string& csv,
    FacialSequence& sequence,
    const MocapCsvOptions& options,
    std::string* error) {

    std::istringstream input(csv);
    std::string line;

    if (!std::getline(input, line)) {
        if (error) *error = "CSV is empty";
        return false;
    }

    const auto headers =
        split(line, options.delimiter);

    if (headers.size() < 2) {
        if (error) *error = "CSV must contain time and at least one curve";
        return false;
    }

    std::size_t timestampIndex = headers.size();
    for (std::size_t i = 0; i < headers.size(); ++i) {
        if (headers[i] == options.timestampColumn) {
            timestampIndex = i;
            break;
        }
    }

    if (timestampIndex == headers.size()) {
        if (error) *error =
            "timestamp column not found: " +
            options.timestampColumn;
        return false;
    }

    FacialSequence out;
    std::size_t row = 1;

    while (std::getline(input, line)) {
        ++row;

        if (trim(line).empty()) {
            continue;
        }

        const auto fields =
            split(line, options.delimiter);

        if (fields.size() != headers.size()) {
            if (error) *error =
                "CSV field count mismatch at row " +
                std::to_string(row);
            return false;
        }

        double timestamp = 0.0;
        if (!parseDouble(fields[timestampIndex], timestamp) ||
            timestamp < 0.0) {
            if (error) *error =
                "invalid timestamp at row " +
                std::to_string(row);
            return false;
        }

        FacialFrame frame;
        frame.timestampSeconds = timestamp;

        for (std::size_t i = 0; i < headers.size(); ++i) {
            if (i == timestampIndex) continue;

            double parsed = 0.0;
            if (!parseDouble(fields[i], parsed)) {
                if (error) *error =
                    "invalid curve value at row " +
                    std::to_string(row) +
                    ", column " + headers[i];
                return false;
            }

            float value =
                static_cast<float>(parsed);

            if (options.clampCurves) {
                value = clamp01(value);
            }

            frame.curves[headers[i]] = value;
        }

        if (!out.addFrame(std::move(frame))) {
            if (error) *error =
                "failed to add frame at row " +
                std::to_string(row);
            return false;
        }
    }

    std::string validationError;
    if (!out.validate(&validationError)) {
        if (error) *error = validationError;
        return false;
    }

    sequence = std::move(out);
    return true;
}

std::string MocapCsv::encode(
    const FacialSequence& sequence,
    char delimiter) {

    std::set<std::string> curveNames;

    for (const auto& frame : sequence.frames()) {
        for (const auto& [id, _] : frame.curves) {
            curveNames.insert(id);
        }
    }

    std::ostringstream out;
    out << std::setprecision(12);
    out << "time";

    for (const auto& id : curveNames) {
        out << delimiter << id;
    }

    out << '\n';

    for (const auto& frame : sequence.frames()) {
        out << frame.timestampSeconds;

        for (const auto& id : curveNames) {
            const auto it = frame.curves.find(id);
            out << delimiter
                << (it == frame.curves.end()
                    ? 0.0F
                    : it->second);
        }

        out << '\n';
    }

    return out.str();
}

} // namespace bdfr::mocap
