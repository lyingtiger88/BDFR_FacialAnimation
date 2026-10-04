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

struct MetaHumanMeshVertex {
    float px = 0.0F;
    float py = 0.0F;
    float pz = 0.0F;
    float nx = 0.0F;
    float ny = 0.0F;
    float nz = 1.0F;
    float u = 0.0F;
    float v = 0.0F;
    std::uint32_t sourcePositionIndex = 0;
};

struct MetaHumanMorphDelta {
    std::uint32_t vertexIndex = 0;
    float dx = 0.0F;
    float dy = 0.0F;
    float dz = 0.0F;
};

struct MetaHumanMorphTarget {
    std::string channelName;
    std::vector<MetaHumanMorphDelta> deltas;
};

struct MetaHumanMeshData {
    std::string name;
    std::vector<MetaHumanMeshVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<MetaHumanMorphTarget> morphTargets;
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
    std::vector<std::string> meshNames() const;
    bool extractMesh(std::uint16_t meshIndex,
                     MetaHumanMeshData& output,
                     std::string* error = nullptr) const;

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
