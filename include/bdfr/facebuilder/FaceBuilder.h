#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bdfr::facebuilder {

struct FaceBuilderLandmark {
    std::string name;
    float x = 0.0F;
    float y = 0.0F;
};

struct FaceBuilderImage {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba;

    bool valid() const noexcept {
        return width > 0 &&
               height > 0 &&
               rgba.size() ==
                   static_cast<std::size_t>(width) *
                   static_cast<std::size_t>(height) *
                   4U;
    }
};

struct FaceBuilderVertex {
    float px = 0.0F;
    float py = 0.0F;
    float pz = 0.0F;

    float nx = 0.0F;
    float ny = 0.0F;
    float nz = 1.0F;

    float u = 0.0F;
    float v = 0.0F;
};

struct FaceBuilderMesh {
    std::vector<FaceBuilderVertex> vertices;
    std::vector<std::uint32_t> indices;
};

struct FaceBuilderResult {
    FaceBuilderMesh mesh;
    FaceBuilderImage texture;

    std::vector<float> shapeCoefficients;

    float yawDegrees = 0.0F;
    float pitchDegrees = 0.0F;
    float rollDegrees = 0.0F;

    std::size_t usedLandmarks = 0;
};

struct FaceBuilderOptions {
    std::string modelPath;
    std::string landmarkMappingPath;

    int textureResolution = 512;
    int shapeCoefficientCount = 40;
    float regularization = 30.0F;
};

class FaceBuilder {
public:
    static bool backendAvailable() noexcept;
    static const char* backendName() noexcept;

    static bool loadPtsLandmarks(
        const std::string& path,
        std::vector<FaceBuilderLandmark>& landmarks,
        std::string* error = nullptr);

    bool fit(
        const FaceBuilderImage& image,
        const std::vector<FaceBuilderLandmark>& landmarks,
        const FaceBuilderOptions& options,
        FaceBuilderResult& result,
        std::string* error = nullptr) const;
};

} // namespace bdfr::facebuilder
