#include "bdfr/facebuilder/FaceBuilder.h"

#include <cstdlib>
#include <iostream>

int main() {
    std::cout
        << "BDFR FaceBuilder backend: "
        << bdfr::facebuilder::FaceBuilder::backendName()
        << "\n";

    return
        bdfr::facebuilder::FaceBuilder::backendAvailable()
            ? EXIT_SUCCESS
            : EXIT_SUCCESS;
}
