#include "bdfr/audio/WavAudio.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>

namespace bdfr::audio {

namespace {

bool readU16(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t& value) {
    if (offset + 2 > bytes.size()) return false;
    value = static_cast<std::uint16_t>(bytes[offset]) |
            static_cast<std::uint16_t>(bytes[offset + 1] << 8);
    return true;
}

bool readU32(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t& value) {
    if (offset + 4 > bytes.size()) return false;
    value = static_cast<std::uint32_t>(bytes[offset]) |
            (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
            (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
            (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
    return true;
}

bool matchFourCC(const std::vector<std::uint8_t>& bytes, std::size_t offset, const char* id) {
    return offset + 4 <= bytes.size() &&
           bytes[offset] == static_cast<std::uint8_t>(id[0]) &&
           bytes[offset + 1] == static_cast<std::uint8_t>(id[1]) &&
           bytes[offset + 2] == static_cast<std::uint8_t>(id[2]) &&
           bytes[offset + 3] == static_cast<std::uint8_t>(id[3]);
}

} // namespace

bool AudioBuffer::valid() const noexcept {
    return sampleRate > 0 && channels > 0 &&
           !samples.empty() &&
           samples.size() % channels == 0;
}

std::size_t AudioBuffer::frameCount() const noexcept {
    return channels == 0 ? 0 : samples.size() / channels;
}

double AudioBuffer::durationSeconds() const noexcept {
    return sampleRate == 0 ? 0.0 :
        static_cast<double>(frameCount()) / static_cast<double>(sampleRate);
}

float AudioBuffer::sample(std::size_t frame, std::size_t channel) const noexcept {
    if (channel >= channels || frame >= frameCount()) return 0.0F;
    return samples[frame * channels + channel];
}

bool WavAudio::decodePcm16(const std::vector<std::uint8_t>& bytes,
                           AudioBuffer& audio,
                           std::string* error) {
    if (bytes.size() < 44 ||
        !matchFourCC(bytes, 0, "RIFF") ||
        !matchFourCC(bytes, 8, "WAVE")) {
        if (error) *error = "not a valid RIFF/WAVE file";
        return false;
    }

    std::uint16_t audioFormat = 0;
    std::uint16_t channels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;
    const std::uint8_t* dataPtr = nullptr;
    std::size_t dataSize = 0;

    std::size_t offset = 12;
    while (offset + 8 <= bytes.size()) {
        std::uint32_t chunkSize = 0;
        if (!readU32(bytes, offset + 4, chunkSize)) break;
        const std::size_t chunkData = offset + 8;
        if (chunkData + chunkSize > bytes.size()) {
            if (error) *error = "truncated WAV chunk";
            return false;
        }

        if (matchFourCC(bytes, offset, "fmt ")) {
            if (chunkSize < 16 ||
                !readU16(bytes, chunkData + 0, audioFormat) ||
                !readU16(bytes, chunkData + 2, channels) ||
                !readU32(bytes, chunkData + 4, sampleRate) ||
                !readU16(bytes, chunkData + 14, bitsPerSample)) {
                if (error) *error = "invalid WAV fmt chunk";
                return false;
            }
        } else if (matchFourCC(bytes, offset, "data")) {
            dataPtr = bytes.data() + chunkData;
            dataSize = chunkSize;
        }

        offset = chunkData + chunkSize + (chunkSize & 1u);
    }

    if (audioFormat != 1 || bitsPerSample != 16 ||
        channels == 0 || sampleRate == 0 || !dataPtr || dataSize == 0) {
        if (error) *error = "only PCM 16-bit WAV is supported";
        return false;
    }
    if (dataSize % 2 != 0) {
        if (error) *error = "invalid PCM16 data size";
        return false;
    }

    AudioBuffer out;
    out.sampleRate = sampleRate;
    out.channels = channels;
    out.samples.resize(dataSize / 2);

    for (std::size_t i = 0; i < out.samples.size(); ++i) {
        const std::uint16_t lo = dataPtr[i * 2];
        const std::uint16_t hi = dataPtr[i * 2 + 1];
        const std::int16_t value =
            static_cast<std::int16_t>(lo | static_cast<std::uint16_t>(hi << 8));
        out.samples[i] = std::clamp(
            static_cast<float>(value) / 32768.0F, -1.0F, 1.0F);
    }

    if (!out.valid()) {
        if (error) *error = "decoded WAV is invalid";
        return false;
    }

    audio = std::move(out);
    return true;
}

bool WavAudio::loadPcm16(const std::filesystem::path& path,
                         AudioBuffer& audio,
                         std::string* error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        if (error) *error = "failed to open WAV file";
        return false;
    }

    std::vector<std::uint8_t> bytes(
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
    if (!in.good() && !in.eof()) {
        if (error) *error = "failed while reading WAV file";
        return false;
    }

    return decodePcm16(bytes, audio, error);
}

} // namespace bdfr::audio
