#include "bdfr/facebuilder/FaceBuilder.h"

#include <eos/core/Landmark.hpp>
#include <eos/core/LandmarkMapper.hpp>
#include <eos/core/read_pts_landmarks.hpp>
#include <eos/fitting/RenderingParameters.hpp>
#include <eos/fitting/linear_shape_fitting.hpp>
#include <eos/fitting/orthographic_camera_estimation_linear.hpp>
#include <eos/morphablemodel/MorphableModel.hpp>
#include <eos/render/normals.hpp>
#include <eos/render/texture_extraction.hpp>

#include <Eigen/Core>

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bdfr::facebuilder {

namespace {

using LandmarkCollection =
    eos::core::LandmarkCollection<Eigen::Vector2f>;

struct RenderVertexKey {
    int positionIndex = 0;
    int textureIndex = 0;

    bool operator==(const RenderVertexKey& other) const noexcept {
        return positionIndex == other.positionIndex &&
               textureIndex == other.textureIndex;
    }
};

struct RenderVertexKeyHash {
    std::size_t operator()(const RenderVertexKey& key) const noexcept {
        return
            (static_cast<std::size_t>(
                 static_cast<std::uint32_t>(key.positionIndex)) << 32U) ^
            static_cast<std::size_t>(
                static_cast<std::uint32_t>(key.textureIndex));
    }
};

eos::core::Image4u toEosImage(
    const FaceBuilderImage& image) {

    eos::core::Image4u output(
        image.height,
        image.width);

    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            const std::size_t offset =
                (
                    static_cast<std::size_t>(y) *
                        static_cast<std::size_t>(image.width) +
                    static_cast<std::size_t>(x)
                ) * 4U;

            output(y, x) =
                eos::core::Pixel<std::uint8_t, 4>(
                    image.rgba[offset + 0],
                    image.rgba[offset + 1],
                    image.rgba[offset + 2],
                    image.rgba[offset + 3]);
        }
    }

    return output;
}

FaceBuilderImage fromEosImage(
    const eos::core::Image4u& image) {

    FaceBuilderImage output;
    output.width = image.width();
    output.height = image.height();

    output.rgba.resize(
        static_cast<std::size_t>(output.width) *
        static_cast<std::size_t>(output.height) *
        4U);

    for (int y = 0; y < output.height; ++y) {
        for (int x = 0; x < output.width; ++x) {
            const auto pixel =
                image(y, x);

            const std::size_t offset =
                (
                    static_cast<std::size_t>(y) *
                        static_cast<std::size_t>(output.width) +
                    static_cast<std::size_t>(x)
                ) * 4U;

            output.rgba[offset + 0] = pixel[0];
            output.rgba[offset + 1] = pixel[1];
            output.rgba[offset + 2] = pixel[2];
            output.rgba[offset + 3] = pixel[3];
        }
    }

    return output;
}

FaceBuilderMesh convertMesh(
    const eos::core::Mesh& mesh) {

    FaceBuilderMesh output;

    if (mesh.vertices.empty() ||
        mesh.tvi.empty()) {
        return output;
    }

    const auto faceNormals =
        eos::render::compute_face_normals(
            mesh.vertices,
            mesh.tvi);

    const auto vertexNormals =
        eos::render::compute_vertex_normals(
            mesh.vertices,
            mesh.tvi,
            faceNormals);

    const bool hasTextureIndices =
        !mesh.tti.empty() &&
        mesh.tti.size() == mesh.tvi.size();

    const bool hasPerVertexUv =
        mesh.texcoords.size() ==
        mesh.vertices.size();

    std::unordered_map<
        RenderVertexKey,
        std::uint32_t,
        RenderVertexKeyHash>
        vertexLookup;

    for (std::size_t triangleIndex = 0;
         triangleIndex < mesh.tvi.size();
         ++triangleIndex) {

        const auto& positionTriangle =
            mesh.tvi[triangleIndex];

        std::array<int, 3> textureTriangle =
            positionTriangle;

        if (hasTextureIndices) {
            textureTriangle =
                mesh.tti[triangleIndex];
        }

        for (int corner = 0;
             corner < 3;
             ++corner) {

            const int positionIndex =
                positionTriangle[corner];

            int textureIndex =
                textureTriangle[corner];

            if (!hasTextureIndices &&
                !hasPerVertexUv) {
                textureIndex = -1;
            }

            RenderVertexKey key{
                positionIndex,
                textureIndex
            };

            const auto found =
                vertexLookup.find(key);

            if (found != vertexLookup.end()) {
                output.indices.push_back(
                    found->second);
                continue;
            }

            FaceBuilderVertex vertex;

            const auto& position =
                mesh.vertices[
                    static_cast<std::size_t>(
                        positionIndex)];

            vertex.px = position.x();
            vertex.py = position.y();
            vertex.pz = position.z();

            if (static_cast<std::size_t>(positionIndex) <
                vertexNormals.size()) {

                const auto& normal =
                    vertexNormals[
                        static_cast<std::size_t>(
                            positionIndex)];

                vertex.nx = normal.x();
                vertex.ny = normal.y();
                vertex.nz = normal.z();
            }

            if (textureIndex >= 0 &&
                static_cast<std::size_t>(textureIndex) <
                    mesh.texcoords.size()) {

                const auto& uv =
                    mesh.texcoords[
                        static_cast<std::size_t>(
                            textureIndex)];

                vertex.u = uv.x();
                vertex.v = uv.y();
            }

            const auto outputIndex =
                static_cast<std::uint32_t>(
                    output.vertices.size());

            output.vertices.push_back(vertex);
            output.indices.push_back(outputIndex);

            vertexLookup.emplace(
                key,
                outputIndex);
        }
    }

    return output;
}

} // namespace

bool FaceBuilder::backendAvailable() noexcept {
    return true;
}

const char* FaceBuilder::backendName() noexcept {
    return "eos 1.5.0";
}

bool FaceBuilder::loadPtsLandmarks(
    const std::string& path,
    std::vector<FaceBuilderLandmark>& landmarks,
    std::string* error) {

    landmarks.clear();

    try {
        const auto source =
            eos::core::read_pts_landmarks(path);

        landmarks.reserve(source.size());

        for (const auto& landmark : source) {
            FaceBuilderLandmark converted;
            converted.name = landmark.name;
            converted.x = landmark.coordinates.x();
            converted.y = landmark.coordinates.y();

            landmarks.push_back(
                std::move(converted));
        }

        if (landmarks.size() < 4) {
            if (error) {
                *error =
                    "The landmark file contains fewer than four usable points.";
            }

            landmarks.clear();
            return false;
        }

        if (error) {
            error->clear();
        }

        return true;

    } catch (const std::exception& exception) {
        if (error) {
            *error = exception.what();
        }

        return false;
    }
}

bool FaceBuilder::fit(
    const FaceBuilderImage& image,
    const std::vector<FaceBuilderLandmark>& landmarks,
    const FaceBuilderOptions& options,
    FaceBuilderResult& result,
    std::string* error) const {

    result = {};

    if (!image.valid()) {
        if (error) {
            *error =
                "Input image is invalid or RGBA data size does not match dimensions.";
        }

        return false;
    }

    if (landmarks.size() < 4) {
        if (error) {
            *error =
                "At least four 2D landmarks are required.";
        }

        return false;
    }

    if (options.modelPath.empty()) {
        if (error) {
            *error =
                "No eos morphable-model file was selected.";
        }

        return false;
    }

    if (options.landmarkMappingPath.empty()) {
        if (error) {
            *error =
                "No eos landmark mapping file was selected.";
        }

        return false;
    }

    try {
        const auto model =
            eos::morphablemodel::load_model(
                options.modelPath);

        const eos::core::LandmarkMapper mapper(
            options.landmarkMappingPath);

        LandmarkCollection eosLandmarks;
        eosLandmarks.reserve(
            landmarks.size());

        for (const auto& landmark : landmarks) {
            eos::core::Landmark<Eigen::Vector2f>
                converted;

            converted.name =
                landmark.name;

            converted.coordinates =
                Eigen::Vector2f(
                    landmark.x,
                    landmark.y);

            eosLandmarks.push_back(
                std::move(converted));
        }

        std::vector<Eigen::Vector4f>
            modelPoints;

        std::vector<int>
            vertexIndices;

        std::vector<Eigen::Vector2f>
            imagePoints;

        const auto meanMesh =
            model.get_mean();

        for (const auto& landmark :
             eosLandmarks) {

            const auto convertedName =
                mapper.convert(
                    landmark.name);

            if (!convertedName) {
                continue;
            }

            int vertexIndex = -1;

            try {
                vertexIndex =
                    std::stoi(
                        convertedName.value());
            } catch (...) {
                continue;
            }

            if (vertexIndex < 0 ||
                static_cast<std::size_t>(
                    vertexIndex) >=
                    meanMesh.vertices.size()) {
                continue;
            }

            modelPoints.emplace_back(
                meanMesh.vertices[
                    static_cast<std::size_t>(
                        vertexIndex)]
                    .homogeneous());

            vertexIndices.push_back(
                vertexIndex);

            imagePoints.push_back(
                landmark.coordinates);
        }

        if (imagePoints.size() < 4) {
            if (error) {
                *error =
                    "Landmark mapping produced fewer than four valid 2D/3D correspondences.";
            }

            return false;
        }

        const auto pose =
            eos::fitting::
                estimate_orthographic_projection_linear(
                    imagePoints,
                    modelPoints,
                    true,
                    image.height);

        eos::fitting::RenderingParameters
            renderingParameters(
                pose,
                image.width,
                image.height);

        const auto affineCamera =
            eos::fitting::
                get_3x4_affine_camera_matrix(
                    renderingParameters,
                    image.width,
                    image.height);

        const int maxCoefficients =
            static_cast<int>(
                model
                    .get_shape_model()
                    .get_num_principal_components());

        const int coefficientCount =
            std::clamp(
                options.shapeCoefficientCount,
                1,
                std::max(1, maxCoefficients));

        const auto fittedCoefficients =
            eos::fitting::
                fit_shape_to_landmarks_linear(
                    model.get_shape_model(),
                    affineCamera,
                    imagePoints,
                    vertexIndices,
                    Eigen::VectorXf(),
                    options.regularization,
                    coefficientCount);

        const auto fittedMesh =
            model.draw_sample(
                fittedCoefficients,
                std::vector<float>());

        const auto texture =
            eos::render::extract_texture(
                fittedMesh,
                renderingParameters.get_modelview(),
                renderingParameters.get_projection(),
                eos::render::ProjectionType::Orthographic,
                toEosImage(image),
                std::max(
                    64,
                    options.textureResolution));

        result.mesh =
            convertMesh(
                fittedMesh);

        result.texture =
            fromEosImage(
                texture);

        result.shapeCoefficients =
            fittedCoefficients;

        const auto angles =
            renderingParameters
                .get_yaw_pitch_roll();

        result.yawDegrees =
            angles[0];

        result.pitchDegrees =
            angles[1];

        result.rollDegrees =
            angles[2];

        result.usedLandmarks =
            imagePoints.size();

        if (result.mesh.vertices.empty() ||
            result.mesh.indices.empty()) {

            if (error) {
                *error =
                    "eos fitting completed but produced an empty mesh.";
            }

            result = {};
            return false;
        }

        if (error) {
            error->clear();
        }

        return true;

    } catch (const std::exception& exception) {
        if (error) {
            *error = exception.what();
        }

        result = {};
        return false;
    }
}

} // namespace bdfr::facebuilder
