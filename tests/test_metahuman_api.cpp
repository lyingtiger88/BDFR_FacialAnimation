#include "bdfr/metahuman/MetaHumanRig.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << "\n";
    } else {
        std::cout << "PASS: " << message << "\n";
    }
}

} // namespace

int main() {
    bdfr::metahuman::MetaHumanRigRuntime runtime;

#if BDFR_HAS_OPENRIGLOGIC
    expect(
        bdfr::metahuman::MetaHumanRigRuntime::backendAvailable(),
        "OpenRigLogic-enabled build exposes MetaHuman backend");
    expect(
        std::string(
            bdfr::metahuman::MetaHumanRigRuntime::backendName())
            .find("OpenRigLogic") != std::string::npos,
        "OpenRigLogic backend reports its implementation name");
#else
    expect(
        !bdfr::metahuman::MetaHumanRigRuntime::backendAvailable(),
        "default build keeps OpenRigLogic optional");
#endif

    expect(
        !runtime.isLoaded(),
        "MetaHuman runtime starts unloaded");

    bdfr::metahuman::MetaHumanMeshData mesh;
    std::string error;

    expect(
        !runtime.extractMesh(0, mesh, &error),
        "mesh extraction rejects missing DNA");

    expect(
        !error.empty(),
        "missing DNA reports a useful error");

    if (failures == 0) {
        std::cout << "All BDFR MetaHuman API tests passed.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << failures << " MetaHuman API test(s) failed.\n";
    return EXIT_FAILURE;
}
