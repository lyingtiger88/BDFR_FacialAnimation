#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace bdfr::models {

enum class RuntimeBackend {
    Unknown,
    Cpu,
    Gpu,
    Npu,
    OnnxRuntime,
    LiteRT,
    MediaPipe
};

struct ModelManifest {
    std::string id;
    std::string version;
    std::string fileName;
    std::string sha256;
    std::string license;
    std::string source;
    RuntimeBackend backend = RuntimeBackend::Unknown;
    std::string inputContract;
    std::string outputContract;
};

class ModelRegistry {
public:
    bool registerModel(ModelManifest model);

    const ModelManifest* find(
        const std::string& id) const;

    const ModelManifest* find(
        const std::string& id,
        const std::string& version) const;

    std::vector<ModelManifest> all() const;

private:
    std::unordered_map<std::string, ModelManifest> models_;
};

} // namespace bdfr::models
