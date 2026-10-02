#pragma once

#include "bdfr/speech/TextSpeech.h"

#include <string>
#include <vector>

namespace bdfr::speech {

struct PhonemeEvent {
    std::string phoneme;
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    float confidence = 1.0F;
};

class IGraphemeToPhonemeProvider {
public:
    virtual ~IGraphemeToPhonemeProvider() = default;

    virtual bool convert(
        const std::string& text,
        const std::string& language,
        std::vector<std::string>& phonemes,
        std::string* error = nullptr) = 0;
};

class IPhonemeAlignmentProvider {
public:
    virtual ~IPhonemeAlignmentProvider() = default;

    virtual bool align(
        const std::string& transcript,
        const std::string& language,
        double audioDurationSeconds,
        std::vector<PhonemeEvent>& phonemes,
        std::string* error = nullptr) = 0;
};

struct TtsTimingResult {
    std::vector<PhonemeEvent> phonemes;
    double durationSeconds = 0.0;
};

class ITtsTimingProvider {
public:
    virtual ~ITtsTimingProvider() = default;

    virtual bool synthesizeTiming(
        const std::string& text,
        const std::string& language,
        const std::string& voice,
        TtsTimingResult& result,
        std::string* error = nullptr) = 0;
};

class IAudioPhonemeProvider {
public:
    virtual ~IAudioPhonemeProvider() = default;

    virtual bool recognize(
        const std::vector<float>& monoSamples,
        std::uint32_t sampleRate,
        const std::string& language,
        std::vector<PhonemeEvent>& phonemes,
        std::string* error = nullptr) = 0;
};

} // namespace bdfr::speech
