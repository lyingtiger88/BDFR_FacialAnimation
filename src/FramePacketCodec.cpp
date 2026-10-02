#include "bdfr/runtime/FramePacketCodec.h"

#include "bdfr/core/BinaryCodec.h"

#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

namespace bdfr::runtime {

namespace {

constexpr std::uint32_t kMagic = 0x50464442U; // BDFP
constexpr std::uint16_t kVersion = 1;
constexpr std::size_t kMaxSourceLength = 1024;
constexpr std::size_t kMaxFrameBytes = 1024 * 1024;

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

} // namespace

std::vector<std::uint8_t> FramePacketCodec::encode(const mocap::MocapPacket& packet) {
    const auto frameBytes = BinaryCodec::encode(packet.frame);

    std::vector<std::uint8_t> out;
    const std::size_t sourceSize = std::min<std::size_t>(
        packet.sourceId.size(), std::numeric_limits<std::uint16_t>::max());
    const auto sourceLength = static_cast<std::uint16_t>(sourceSize);
    const auto frameLength = static_cast<std::uint32_t>(frameBytes.size());

    append(out, kMagic);
    append(out, kVersion);
    append(out, packet.sequenceNumber);
    append(out, sourceLength);
    out.insert(out.end(), packet.sourceId.begin(), packet.sourceId.begin() + sourceLength);
    append(out, frameLength);
    out.insert(out.end(), frameBytes.begin(), frameBytes.end());
    return out;
}

bool FramePacketCodec::decode(const std::vector<std::uint8_t>& bytes,
                              mocap::MocapPacket& packet) {
    std::size_t offset = 0;
    std::uint32_t magic = 0;
    std::uint16_t version = 0;
    std::uint64_t sequence = 0;
    std::uint16_t sourceLength = 0;
    std::uint32_t frameLength = 0;

    if (!read(bytes, offset, magic) || magic != kMagic ||
        !read(bytes, offset, version) || version != kVersion ||
        !read(bytes, offset, sequence) ||
        !read(bytes, offset, sourceLength) ||
        sourceLength > kMaxSourceLength ||
        offset + sourceLength > bytes.size()) {
        return false;
    }

    std::string source(
        reinterpret_cast<const char*>(bytes.data() + offset), sourceLength);
    offset += sourceLength;

    if (!read(bytes, offset, frameLength) ||
        frameLength > kMaxFrameBytes ||
        offset + frameLength != bytes.size()) {
        return false;
    }

    std::vector<std::uint8_t> frameBytes(
        bytes.begin() + static_cast<std::ptrdiff_t>(offset), bytes.end());

    FacialFrame frame;
    if (!BinaryCodec::decode(frameBytes, frame)) return false;

    packet.sourceId = std::move(source);
    packet.sequenceNumber = sequence;
    packet.frame = std::move(frame);
    return true;
}

} // namespace bdfr::runtime
