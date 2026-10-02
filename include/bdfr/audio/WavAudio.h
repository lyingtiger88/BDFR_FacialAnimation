#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace bdfr::audio {

struct AudioBuffer {
    std::uint32_t sampleRate = 0;
    std::uint16_t channels = 0;
    std::vector<float> samples; // interleaved, normalized [-1,1]

    bool valid() const noexcept;
    std::size_t frameCount() const noexcept;
    double durationSeconds() const noexcept;
    float sample(std::size_t frame, std::size_t channel) const noexcept;
};

class WavAudio {
public:
    static bool decodePcm16(const std::vector<std::uint8_t>& bytes,
                            AudioBuffer& audio,
                            std::string* error = nullptr);

    static bool loadPcm16(const std::filesystem::path& path,
                          AudioBuffer& audio,
                          std::string* error = nullptr);
};

} // namespace bdfr::audio
