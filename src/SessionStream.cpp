#include "bdfr/runtime/SessionStream.h"

#include "bdfr/runtime/FramePacketCodec.h"

#include <fstream>
#include <iterator>

namespace bdfr::runtime {

namespace {
constexpr std::uint32_t Magic = 0x53464442U; // BDFS
constexpr std::uint16_t Version = 1;
constexpr std::size_t MaxPacketBytes = 1024 * 1024;
constexpr std::size_t MaxPackets = 10'000'000;

void appendU16(std::vector<std::uint8_t>& out, std::uint16_t value) {
    out.push_back(static_cast<std::uint8_t>(value & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
}

void appendU32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i)
        out.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFu));
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
    for (int i = 0; i < 4; ++i)
        value |= static_cast<std::uint32_t>(bytes[offset + i]) << (i * 8);
    offset += 4;
    return true;
}
} // namespace

std::vector<std::uint8_t> SessionStream::encode(
    const std::vector<mocap::MocapPacket>& packets) {

    std::vector<std::uint8_t> out;
    appendU32(out, Magic);
    appendU16(out, Version);
    appendU32(out, static_cast<std::uint32_t>(packets.size()));

    for (const auto& packet : packets) {
        const auto encoded = FramePacketCodec::encode(packet);
        appendU32(out, static_cast<std::uint32_t>(encoded.size()));
        out.insert(out.end(), encoded.begin(), encoded.end());
    }

    return out;
}

bool SessionStream::decode(
    const std::vector<std::uint8_t>& bytes,
    std::vector<mocap::MocapPacket>& packets,
    std::string* error) {

    std::size_t offset = 0;
    std::uint32_t magic = 0;
    std::uint16_t version = 0;
    std::uint32_t count = 0;

    if (!readU32(bytes, offset, magic) || magic != Magic ||
        !readU16(bytes, offset, version) || version != Version ||
        !readU32(bytes, offset, count) || count > MaxPackets) {
        if (error) *error = "invalid BDFS session header";
        return false;
    }

    std::vector<mocap::MocapPacket> out;
    out.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t length = 0;
        if (!readU32(bytes, offset, length) ||
            length == 0 || length > MaxPacketBytes ||
            offset + length > bytes.size()) {
            if (error) *error = "invalid BDFS packet length";
            return false;
        }

        std::vector<std::uint8_t> packetBytes(
            bytes.begin() + static_cast<std::ptrdiff_t>(offset),
            bytes.begin() + static_cast<std::ptrdiff_t>(offset + length));
        offset += length;

        mocap::MocapPacket packet;
        if (!FramePacketCodec::decode(packetBytes, packet)) {
            if (error) *error = "invalid packet inside BDFS session";
            return false;
        }
        out.push_back(std::move(packet));
    }

    if (offset != bytes.size()) {
        if (error) *error = "trailing data in BDFS session";
        return false;
    }

    packets = std::move(out);
    return true;
}

bool SessionStream::save(
    const std::filesystem::path& path,
    const std::vector<mocap::MocapPacket>& packets,
    std::string* error) {

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        if (error) *error = "failed to open BDFS file for writing";
        return false;
    }

    const auto bytes = encode(packets);
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));

    if (!out) {
        if (error) *error = "failed while writing BDFS file";
        return false;
    }
    return true;
}

bool SessionStream::load(
    const std::filesystem::path& path,
    std::vector<mocap::MocapPacket>& packets,
    std::string* error) {

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        if (error) *error = "failed to open BDFS file for reading";
        return false;
    }

    std::vector<std::uint8_t> bytes{
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()};

    return decode(bytes, packets, error);
}

} // namespace bdfr::runtime
