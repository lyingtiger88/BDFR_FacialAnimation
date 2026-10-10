#include "bdfr/facebuilder/FaceBuilder.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#ifndef BDFR_EOS_TEST_DATA_DIR
#error BDFR_EOS_TEST_DATA_DIR must be provided by CMake
#endif

int main() {
    using namespace bdfr::facebuilder;

    if (!FaceBuilder::backendAvailable()) {
        std::cerr << "FaceBuilder backend unavailable.\n";
        return EXIT_FAILURE;
    }

    const std::string root = BDFR_EOS_TEST_DATA_DIR;

    std::vector<FaceBuilderLandmark> landmarks;
    std::string error;

    if (!FaceBuilder::loadPtsLandmarks(
            root + "/examples/data/image_0010.pts",
            landmarks,
            &error)) {
        std::cerr << "Failed to load eos landmarks: "
                  << error << "\n";
        return EXIT_FAILURE;
    }

    FaceBuilderImage image;
    image.width = 1024;
    image.height = 768;
    image.rgba.resize(
        static_cast<std::size_t>(image.width) *
        static_cast<std::size_t>(image.height) *
        4U,
        180U);

    for (std::size_t i = 3;
         i < image.rgba.size();
         i += 4) {
        image.rgba[i] = 255U;
    }

    FaceBuilderOptions options;
    options.modelPath =
        root + "/share/sfm_shape_3448.bin";
    options.landmarkMappingPath =
        root + "/share/ibug_to_sfm.txt";
    options.textureResolution = 128;
    options.shapeCoefficientCount = 20;
    options.regularization = 30.0F;

    FaceBuilder builder;
    FaceBuilderResult result;

    if (!builder.fit(
            image,
            landmarks,
            options,
            result,
            &error)) {
        std::cerr << "FaceBuilder fit failed: "
                  << error << "\n";
        return EXIT_FAILURE;
    }

    if (result.mesh.vertices.empty() ||
        result.mesh.indices.empty()) {
        std::cerr << "FaceBuilder produced empty mesh.\n";
        return EXIT_FAILURE;
    }

    if (!result.texture.valid()) {
        std::cerr << "FaceBuilder produced invalid texture.\n";
        return EXIT_FAILURE;
    }

    std::cout
        << "FaceBuilder integration passed: "
        << result.usedLandmarks
        << " landmarks, "
        << result.mesh.vertices.size()
        << " render vertices, "
        << result.mesh.indices.size() / 3
        << " triangles, texture "
        << result.texture.width
        << "x"
        << result.texture.height
        << "\n";

    return EXIT_SUCCESS;
}
