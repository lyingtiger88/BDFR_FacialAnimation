#include "bdfr/core/BinaryCodec.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>

namespace bdfr {

namespace {
constexpr std::uint32_t kMagic = 0x52464442U;
constexpr std::uint16_t kCodecVersion = 1;
constexpr std::size_t kMaxCurves = 4096;
constexpr std::size_t kMaxStringLength = 1024;

void appendU16(std::vector<std::uint8_t>& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
}
void appendU32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFu));
}
void appendU64(std::vector<std::uint8_t>& out, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) out.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFu));
}
void appendFloat(std::vector<std::uint8_t>& out, float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    appendU32(out, bits);
}
void appendDouble(std::vector<std::uint8_t>& out, double value) {
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    appendU64(out, bits);
}
bool readU16(const std::vector<std::uint8_t>& bytes, std::size_t& offset, std::uint16_t& value) {
    if (offset + 2 > bytes.size()) return false;
    value = static_cast<std::uint16_t>(bytes[offset]) |
            static_cast<std::uint16_t>(bytes[offset + 1] << 8);
    offset += 2;
    return true;
}
bool readU32(const std::vector<std::uint8_t>& bytes, std::size_t& offset, std::uint32_t& value) {
    if (offset + 4 > bytes.size()) return false;
    value = 0;
    for (int i = 0; i < 4; ++i) value |= static_cast<std::uint32_t>(bytes[offset + i]) << (i * 8);
    offset += 4;
    return true;
}
bool readU64(const std::vector<std::uint8_t>& bytes, std::size_t& offset, std::uint64_t& value) {
    if (offset + 8 > bytes.size()) return false;
    value = 0;
    for (int i = 0; i < 8; ++i) value |= static_cast<std::uint64_t>(bytes[offset + i]) << (i * 8);
    offset += 8;
    return true;
}
bool readFloat(const std::vector<std::uint8_t>& bytes, std::size_t& offset, float& value) {
    std::uint32_t bits = 0;
    if (!readU32(bytes, offset, bits)) return false;
    std::memcpy(&value, &bits, sizeof(value));
    return true;
}
bool readDouble(const std::vector<std::uint8_t>& bytes, std::size_t& offset, double& value) {
    std::uint64_t bits = 0;
    if (!readU64(bytes, offset, bits)) return false;
    std::memcpy(&value, &bits, sizeof(value));
    return true;
}
void appendString(std::vector<std::uint8_t>& out, const std::string& value) {
    const auto length = static_cast<std::uint16_t>(
        std::min<std::size_t>(value.size(), std::numeric_limits<std::uint16_t>::max()));
    appendU16(out, length);
    out.insert(out.end(), value.begin(), value.begin() + length);
}
bool readString(const std::vector<std::uint8_t>& bytes, std::size_t& offset, std::string& value) {
    std::uint16_t length = 0;
    if (!readU16(bytes, offset, length) || length > kMaxStringLength || offset + length > bytes.size()) return false;
    value.assign(reinterpret_cast<const char*>(bytes.data() + offset), length);
    offset += length;
    return true;
}
} // namespace

std::vector<std::uint8_t> BinaryCodec::encode(const FacialFrame& frame) {
    std::vector<std::uint8_t> out;
    out.reserve(64 + frame.curves.size() * 24);

    appendU32(out, kMagic);
    appendU16(out, kCodecVersion);
    appendU32(out, frame.schemaVersion);
    appendDouble(out, frame.timestampSeconds);
    appendFloat(out, frame.confidence);
    appendFloat(out, frame.head.pitch);
    appendFloat(out, frame.head.yaw);
    appendFloat(out, frame.head.roll);
    appendFloat(out, frame.gaze.x);
    appendFloat(out, frame.gaze.y);
    appendFloat(out, frame.gaze.confidence);

    std::vector<std::pair<std::string, float>> sorted(frame.curves.begin(), frame.curves.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    appendU32(out, static_cast<std::uint32_t>(sorted.size()));
    for (const auto& [id, value] : sorted) {
        appendString(out, id);
        appendFloat(out, value);
    }
    return out;
}

bool BinaryCodec::decode(const std::vector<std::uint8_t>& bytes, FacialFrame& outFrame) {
    std::size_t offset = 0;
    std::uint32_t magic = 0;
    std::uint16_t codecVersion = 0;
    FacialFrame frame;

    if (!readU32(bytes, offset, magic) || magic != kMagic ||
        !readU16(bytes, offset, codecVersion) || codecVersion != kCodecVersion ||
        !readU32(bytes, offset, frame.schemaVersion) ||
        !readDouble(bytes, offset, frame.timestampSeconds) ||
        !readFloat(bytes, offset, frame.confidence) ||
        !readFloat(bytes, offset, frame.head.pitch) ||
        !readFloat(bytes, offset, frame.head.yaw) ||
        !readFloat(bytes, offset, frame.head.roll) ||
        !readFloat(bytes, offset, frame.gaze.x) ||
        !readFloat(bytes, offset, frame.gaze.y) ||
        !readFloat(bytes, offset, frame.gaze.confidence)) return false;

    std::uint32_t count = 0;
    if (!readU32(bytes, offset, count) || count > kMaxCurves) return false;
    for (std::uint32_t i = 0; i < count; ++i) {
        std::string id;
        float value = 0.0F;
        if (!readString(bytes, offset, id) || !readFloat(bytes, offset, value)) return false;
        frame.curves[id] = value;
    }

    if (offset != bytes.size()) return false;
    outFrame = std::move(frame);
    return true;
}

} // namespace bdfr
