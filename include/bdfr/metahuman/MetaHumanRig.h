#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bdfr::metahuman {

struct MetaHumanRigInfo {
    std::string characterName;
    std::string databaseName;
    std::uint16_t lodCount = 0;
    std::uint16_t guiControlCount = 0;
    std::uint16_t rawControlCount = 0;
    std::uint16_t jointCount = 0;
    std::uint16_t blendShapeChannelCount = 0;
    std::uint16_t animatedMapCount = 0;
    std::uint16_t meshCount = 0;
};

struct MetaHumanRigOutput {
    std::vector<float> jointValues;
    std::unordered_map<std::string, float> blendShapes;
    std::unordered_map<std::string, float> animatedMaps;

    std::size_t mappedInputCurves = 0;
    std::size_t rawControlCount = 0;
};

class MetaHumanRigRuntime {
public:
    MetaHumanRigRuntime();
    ~MetaHumanRigRuntime();

    MetaHumanRigRuntime(const MetaHumanRigRuntime&) = delete;
    MetaHumanRigRuntime& operator=(const MetaHumanRigRuntime&) = delete;

    MetaHumanRigRuntime(MetaHumanRigRuntime&&) noexcept;
    MetaHumanRigRuntime& operator=(MetaHumanRigRuntime&&) noexcept;

    static bool backendAvailable() noexcept;
    static const char* backendName() noexcept;

    bool loadDna(const std::string& path, std::string* error = nullptr);
    void unload();

    bool isLoaded() const noexcept;
    const MetaHumanRigInfo& info() const noexcept;

    std::vector<std::string> rawControlNames() const;

    bool evaluate(const CurveMap& curves,
                  MetaHumanRigOutput& output,
                  std::string* error = nullptr);

    bool evaluate(const FacialFrame& frame,
                  MetaHumanRigOutput& output,
                  std::string* error = nullptr);

    std::vector<std::pair<std::string, std::uint16_t>>
    resolveMappings(const CurveMap& curves) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bdfr::metahuman
