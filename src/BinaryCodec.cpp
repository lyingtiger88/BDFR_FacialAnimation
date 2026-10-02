#include "bdfr/core/BinaryCodec.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

namespace bdfr {

namespace {
constexpr std::uint32_t kMagic = 0x52464442U;
constexpr std::uint16_t kCodecVersion = 1;
constexpr std::size_t kMaxCurves = 4096;
constexpr std::size_t kMaxStringLength = 1024;

template <typename T>
void append(std::vector<std::uint8_t>& out, const T& value) {
    static_assert(std::is_trivially_copyable_v<T>);
    const auto* ptr = reinterpret_cast<const std::uint8_t*>(&value);
    out.insert(out.end(), ptr, ptr + sizeof(T));
}

template <typename T>
bool read(const std::vector<std::uint8_t>& bytes, std::size_t& offset, T& value) {
    static_assert(std::is_trivially_copyable_v<T>);
    if (offset + sizeof(T) > bytes.size()) return false;
    std::memcpy(&value, bytes.data() + offset, sizeof(T));
    offset += sizeof(T);
    return true;
}

void appendString(std::vector<std::uint8_t>& out, const std::string& value) {
    const auto length = static_cast<std::uint16_t>(
        std::min<std::size_t>(value.size(), std::numeric_limits<std::uint16_t>::max()));
    append(out, length);
    out.insert(out.end(), value.begin(), value.begin() + length);
}

bool readString(const std::vector<std::uint8_t>& bytes, std::size_t& offset, std::string& value) {
    std::uint16_t length = 0;
    if (!read(bytes, offset, length) || length > kMaxStringLength || offset + length > bytes.size())
        return false;
    value.assign(reinterpret_cast<const char*>(bytes.data() + offset), length);
    offset += length;
    return true;
}
} // namespace

std::vector<std::uint8_t> BinaryCodec::encode(const FacialFrame& frame) {
    std::vector<std::uint8_t> out;
    out.reserve(64 + frame.curves.size() * 24);

    append(out, kMagic);
    append(out, kCodecVersion);
    append(out, frame.schemaVersion);
    append(out, frame.timestampSeconds);
    append(out, frame.confidence);
    append(out, frame.head.pitch);
    append(out, frame.head.yaw);
    append(out, frame.head.roll);
    append(out, frame.gaze.x);
    append(out, frame.gaze.y);
    append(out, frame.gaze.confidence);

    std::vector<std::pair<std::string, float>> sorted(frame.curves.begin(), frame.curves.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    const auto count = static_cast<std::uint32_t>(sorted.size());
    append(out, count);
    for (const auto& [id, value] : sorted) {
        appendString(out, id);
        append(out, value);
    }
    return out;
}

bool BinaryCodec::decode(const std::vector<std::uint8_t>& bytes, FacialFrame& outFrame) {
    std::size_t offset = 0;
    std::uint32_t magic = 0;
    std::uint16_t codecVersion = 0;
    FacialFrame frame;

    if (!read(bytes, offset, magic) || magic != kMagic ||
        !read(bytes, offset, codecVersion) || codecVersion != kCodecVersion ||
        !read(bytes, offset, frame.schemaVersion) ||
        !read(bytes, offset, frame.timestampSeconds) ||
        !read(bytes, offset, frame.confidence) ||
        !read(bytes, offset, frame.head.pitch) ||
        !read(bytes, offset, frame.head.yaw) ||
        !read(bytes, offset, frame.head.roll) ||
        !read(bytes, offset, frame.gaze.x) ||
        !read(bytes, offset, frame.gaze.y) ||
        !read(bytes, offset, frame.gaze.confidence)) return false;

    std::uint32_t count = 0;
    if (!read(bytes, offset, count) || count > kMaxCurves) return false;

    for (std::uint32_t i = 0; i < count; ++i) {
        std::string id;
        float value = 0.0F;
        if (!readString(bytes, offset, id) || !read(bytes, offset, value)) return false;
        frame.curves[id] = value;
    }

    if (offset != bytes.size()) return false;
    outFrame = std::move(frame);
    return true;
}

} // namespace bdfr
