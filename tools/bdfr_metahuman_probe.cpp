#include "bdfr/metahuman/MetaHumanRig.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    using bdfr::metahuman::MetaHumanRigRuntime;

    std::cout << "BDFR MetaHuman Rig backend: "
              << MetaHumanRigRuntime::backendName() << "\n";

    if (!MetaHumanRigRuntime::backendAvailable()) {
        std::cerr << "OpenRigLogic backend is not available in this build.\n";
        return argc <= 1 ? EXIT_SUCCESS : 2;
    }

    if (argc <= 1) {
        std::cout << "Usage: bdfr_metahuman_probe <character.dna>\n";
        return EXIT_SUCCESS;
    }

    MetaHumanRigRuntime runtime;
    std::string error;

    if (!runtime.loadDna(argv[1], &error)) {
        std::cerr << "DNA load failed: " << error << "\n";
        return 3;
    }

    const auto& info = runtime.info();

    std::cout
        << "Character: " << info.characterName << "\n"
        << "Database: " << info.databaseName << "\n"
        << "LODs: " << info.lodCount << "\n"
        << "GUI controls: " << info.guiControlCount << "\n"
        << "Raw controls: " << info.rawControlCount << "\n"
        << "Joints: " << info.jointCount << "\n"
        << "Blendshape channels: " << info.blendShapeChannelCount << "\n"
        << "Animated maps: " << info.animatedMapCount << "\n"
        << "Meshes: " << info.meshCount << "\n";

    return EXIT_SUCCESS;
}
