#include "bdfr/metahuman/MetaHumanRig.h"

#include <dna/BinaryStreamReader.h>
#include <riglogic/RigLogic.h>
#include <status/Status.h>
#include <trio/streams/FileStream.h>

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bdfr::metahuman {

namespace {

std::string statusMessage(const char* fallback) {
    if (sc::Status::isOk()) {
        return fallback ? fallback : "OpenRigLogic error";
    }

    const auto status = sc::Status::get();
    if (status.message && status.message[0] != '\0') {
        return status.message;
    }

    return fallback ? fallback : "OpenRigLogic error";
}

std::string normalizeName(const std::string& input) {
    std::string out;
    out.reserve(input.size());

    for (unsigned char ch : input) {
        if (std::isalnum(ch)) {
            out.push_back(static_cast<char>(std::tolower(ch)));
        }
    }

    return out;
}

std::vector<std::string> curveVariants(const std::string& input) {
    const std::string normalized = normalizeName(input);

    std::vector<std::string> variants;
    variants.push_back(normalized);

    auto replaceAll = [](std::string value,
                         const std::string& from,
                         const std::string& to) {
        std::size_t pos = 0;
        while ((pos = value.find(from, pos)) != std::string::npos) {
            value.replace(pos, from.size(), to);
            pos += to.size();
        }
        return value;
    };

    const std::string lr =
        replaceAll(
            replaceAll(normalized, "left", "l"),
            "right",
            "r");

    if (lr != normalized) {
        variants.push_back(lr);
    }

    const std::string sideWords =
        replaceAll(
            replaceAll(normalized, "left", ""),
            "right",
            "");

    if (!sideWords.empty() &&
        sideWords != normalized &&
        sideWords != lr) {
        variants.push_back(sideWords);
    }

    return variants;
}

int mappingScore(const std::string& rawName,
                 const std::vector<std::string>& variants) {
    const std::string raw = normalizeName(rawName);
    int best = -1;

    for (const std::string& variant : variants) {
        if (variant.empty()) {
            continue;
        }

        if (raw == variant) {
            best = std::max(best, 1000);
            continue;
        }

        if (raw.size() >= variant.size() &&
            raw.compare(raw.size() - variant.size(), variant.size(), variant) == 0) {
            best = std::max(
                best,
                900 - static_cast<int>(raw.size() - variant.size()));
            continue;
        }

        const auto pos = raw.find(variant);
        if (pos != std::string::npos) {
            best = std::max(
                best,
                700 - static_cast<int>(raw.size() - variant.size()));
        }
    }

    return best;
}

} // namespace

struct MetaHumanRigRuntime::Impl {
    trio::FileStream* stream = nullptr;
    dna::BinaryStreamReader* reader = nullptr;
    rl4::RigLogic* rigLogic = nullptr;
    rl4::RigInstance* rigInstance = nullptr;

    MetaHumanRigInfo info;
    std::vector<std::string> rawNames;
    std::vector<std::string> blendShapeNames;
    std::vector<std::string> animatedMapNames;

    void reset() {
        if (rigInstance) {
            rl4::RigInstance::destroy(rigInstance);
            rigInstance = nullptr;
        }

        if (rigLogic) {
            rl4::RigLogic::destroy(rigLogic);
            rigLogic = nullptr;
        }

        if (reader) {
            dna::BinaryStreamReader::destroy(reader);
            reader = nullptr;
        }

        if (stream) {
            trio::FileStream::destroy(stream);
            stream = nullptr;
        }

        info = {};
        rawNames.clear();
        blendShapeNames.clear();
        animatedMapNames.clear();
    }

    ~Impl() {
        reset();
    }
};

MetaHumanRigRuntime::MetaHumanRigRuntime()
    : impl_(std::make_unique<Impl>()) {}

MetaHumanRigRuntime::~MetaHumanRigRuntime() = default;

MetaHumanRigRuntime::MetaHumanRigRuntime(MetaHumanRigRuntime&&) noexcept = default;
MetaHumanRigRuntime& MetaHumanRigRuntime::operator=(MetaHumanRigRuntime&&) noexcept = default;

bool MetaHumanRigRuntime::backendAvailable() noexcept {
    return true;
}

const char* MetaHumanRigRuntime::backendName() noexcept {
    return "Epic OpenRigLogic 5.8";
}

bool MetaHumanRigRuntime::loadDna(
    const std::string& path,
    std::string* error) {

    impl_->reset();

    if (path.empty()) {
        if (error) {
            *error = "DNA path is empty.";
        }
        return false;
    }

    impl_->stream = trio::FileStream::create(
        path.c_str(),
        trio::FileStream::AccessMode::Read,
        trio::FileStream::OpenMode::Binary);

    if (!impl_->stream || !sc::Status::isOk()) {
        if (error) {
            *error = statusMessage("Unable to open MetaHuman DNA file.");
        }
        impl_->reset();
        return false;
    }

    dna::Configuration dnaConfig{};
    dnaConfig.layer = dna::DataLayer::All;
    dnaConfig.unknownLayerPolicy = dna::UnknownLayerPolicy::Ignore;

    impl_->reader = dna::BinaryStreamReader::create(
        impl_->stream,
        dnaConfig);

    if (!impl_->reader) {
        if (error) {
            *error = "Unable to create DNA reader.";
        }
        impl_->reset();
        return false;
    }

    impl_->reader->read();

    if (!sc::Status::isOk()) {
        if (error) {
            *error = statusMessage("MetaHuman DNA read failed.");
        }
        impl_->reset();
        return false;
    }

    rl4::Configuration rigConfig{};
    rigConfig.loadJoints = true;
    rigConfig.loadBlendShapes = true;
    rigConfig.loadAnimatedMaps = true;
    rigConfig.loadMachineLearnedBehavior = true;
    rigConfig.loadRBFBehavior = true;
    rigConfig.loadTwistSwingBehavior = true;

    impl_->rigLogic = rl4::RigLogic::create(
        impl_->reader,
        rigConfig);

    if (!impl_->rigLogic || !sc::Status::isOk()) {
        if (error) {
            *error = statusMessage("Unable to initialize RigLogic.");
        }
        impl_->reset();
        return false;
    }

    impl_->rigInstance =
        rl4::RigInstance::create(impl_->rigLogic);

    if (!impl_->rigInstance) {
        if (error) {
            *error = "Unable to create RigLogic instance.";
        }
        impl_->reset();
        return false;
    }

    impl_->info.characterName =
        impl_->reader->getName().c_str();

    impl_->info.databaseName =
        impl_->reader->getDBName().c_str();

    impl_->info.lodCount =
        impl_->rigLogic->getLODCount();

    impl_->info.guiControlCount =
        impl_->reader->getGUIControlCount();

    impl_->info.rawControlCount =
        impl_->reader->getRawControlCount();

    impl_->info.jointCount =
        impl_->reader->getJointCount();

    impl_->info.blendShapeChannelCount =
        impl_->reader->getBlendShapeChannelCount();

    impl_->info.animatedMapCount =
        impl_->reader->getAnimatedMapCount();

    impl_->info.meshCount =
        impl_->reader->getMeshCount();

    impl_->rawNames.reserve(
        impl_->info.rawControlCount);

    for (std::uint16_t i = 0;
         i < impl_->info.rawControlCount;
         ++i) {
        impl_->rawNames.emplace_back(
            impl_->reader->getRawControlName(i).c_str());
    }

    impl_->blendShapeNames.reserve(
        impl_->info.blendShapeChannelCount);

    for (std::uint16_t i = 0;
         i < impl_->info.blendShapeChannelCount;
         ++i) {
        impl_->blendShapeNames.emplace_back(
            impl_->reader->getBlendShapeChannelName(i).c_str());
    }

    impl_->animatedMapNames.reserve(
        impl_->info.animatedMapCount);

    for (std::uint16_t i = 0;
         i < impl_->info.animatedMapCount;
         ++i) {
        impl_->animatedMapNames.emplace_back(
            impl_->reader->getAnimatedMapName(i).c_str());
    }

    if (impl_->info.lodCount > 0) {
        impl_->rigInstance->setLOD(0);
    }

    if (error) {
        error->clear();
    }

    return true;
}

void MetaHumanRigRuntime::unload() {
    impl_->reset();
}

bool MetaHumanRigRuntime::isLoaded() const noexcept {
    return impl_->rigInstance != nullptr &&
           impl_->rigLogic != nullptr &&
           impl_->reader != nullptr;
}

const MetaHumanRigInfo& MetaHumanRigRuntime::info() const noexcept {
    return impl_->info;
}

std::vector<std::string>
MetaHumanRigRuntime::rawControlNames() const {
    return impl_->rawNames;
}

std::vector<std::pair<std::string, std::uint16_t>>
MetaHumanRigRuntime::resolveMappings(
    const CurveMap& curves) const {

    std::vector<std::pair<std::string, std::uint16_t>> result;

    if (!isLoaded()) {
        return result;
    }

    result.reserve(curves.size());

    for (const auto& [curveName, ignored] : curves) {
        (void)ignored;

        const auto variants =
            curveVariants(curveName);

        int bestScore = -1;
        std::uint16_t bestIndex = 0;

        for (std::uint16_t i = 0;
             i < impl_->rawNames.size();
             ++i) {
            const int score =
                mappingScore(
                    impl_->rawNames[i],
                    variants);

            if (score > bestScore) {
                bestScore = score;
                bestIndex = i;
            }
        }

        if (bestScore >= 650) {
            result.emplace_back(
                curveName,
                bestIndex);
        }
    }

    return result;
}

bool MetaHumanRigRuntime::evaluate(
    const CurveMap& curves,
    MetaHumanRigOutput& output,
    std::string* error) {

    output = {};

    if (!isLoaded()) {
        if (error) {
            *error = "No MetaHuman DNA is loaded.";
        }
        return false;
    }

    for (std::uint16_t i = 0;
         i < impl_->rigInstance->getRawControlCount();
         ++i) {
        impl_->rigInstance->setRawControl(i, 0.0F);
    }

    const auto mappings =
        resolveMappings(curves);

    for (const auto& [curveName, rawIndex] : mappings) {
        const auto it = curves.find(curveName);
        if (it == curves.end()) {
            continue;
        }

        impl_->rigInstance->setRawControl(
            rawIndex,
            std::clamp(it->second, 0.0F, 1.0F));
    }

    impl_->rigLogic->calculate(
        impl_->rigInstance);

    if (!sc::Status::isOk()) {
        if (error) {
            *error = statusMessage("RigLogic evaluation failed.");
        }
        return false;
    }

    const auto jointOutputs =
        impl_->rigInstance->getJointOutputs();

    output.jointValues.assign(
        jointOutputs.begin(),
        jointOutputs.end());

    const auto blendOutputs =
        impl_->rigInstance->getBlendShapeOutputs();

    const std::size_t blendCount =
        std::min<std::size_t>(
            blendOutputs.size(),
            impl_->blendShapeNames.size());

    output.blendShapes.reserve(blendCount);

    for (std::size_t i = 0; i < blendCount; ++i) {
        output.blendShapes.emplace(
            impl_->blendShapeNames[i],
            blendOutputs[i]);
    }

    const auto animatedOutputs =
        impl_->rigInstance->getAnimatedMapOutputs();

    const std::size_t animatedCount =
        std::min<std::size_t>(
            animatedOutputs.size(),
            impl_->animatedMapNames.size());

    output.animatedMaps.reserve(animatedCount);

    for (std::size_t i = 0; i < animatedCount; ++i) {
        output.animatedMaps.emplace(
            impl_->animatedMapNames[i],
            animatedOutputs[i]);
    }

    output.mappedInputCurves =
        mappings.size();

    output.rawControlCount =
        impl_->rigInstance->getRawControlCount();

    if (error) {
        error->clear();
    }

    return true;
}

bool MetaHumanRigRuntime::evaluate(
    const FacialFrame& frame,
    MetaHumanRigOutput& output,
    std::string* error) {
    return evaluate(frame.curves, output, error);
}

} // namespace bdfr::metahuman
