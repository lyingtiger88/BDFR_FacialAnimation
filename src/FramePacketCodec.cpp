#include "bdfr/runtime/FramePacketCodec.h"

#include "bdfr/core/BinaryCodec.h"

#include <limits>
#include <string>

namespace bdfr::runtime {

namespace {
constexpr std::uint32_t kMagic = 0x50464442U;
constexpr std::uint16_t kVersion = 1;
constexpr std::size_t kMaxSourceLength = 1024;
constexpr std::size_t kMaxFrameBytes = 1024 * 1024;

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
} // namespace

std::vector<std::uint8_t> FramePacketCodec::encode(const mocap::MocapPacket& packet) {
    const auto frameBytes = BinaryCodec::encode(packet.frame);
    std::vector<std::uint8_t> out;

    const std::size_t sourceSize = std::min<std::size_t>(
        packet.sourceId.size(), std::numeric_limits<std::uint16_t>::max());
    const auto sourceLength = static_cast<std::uint16_t>(sourceSize);

    appendU32(out, kMagic);
    appendU16(out, kVersion);
    appendU64(out, packet.sequenceNumber);
    appendU16(out, sourceLength);
    out.insert(out.end(), packet.sourceId.begin(), packet.sourceId.begin() + sourceLength);
    appendU32(out, static_cast<std::uint32_t>(frameBytes.size()));
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

    if (!readU32(bytes, offset, magic) || magic != kMagic ||
        !readU16(bytes, offset, version) || version != kVersion ||
        !readU64(bytes, offset, sequence) ||
        !readU16(bytes, offset, sourceLength) ||
        sourceLength > kMaxSourceLength ||
        offset + sourceLength > bytes.size()) return false;

    std::string source(reinterpret_cast<const char*>(bytes.data() + offset), sourceLength);
    offset += sourceLength;

    if (!readU32(bytes, offset, frameLength) ||
        frameLength > kMaxFrameBytes ||
        offset + frameLength != bytes.size()) return false;

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
