#pragma once

#include "bdfr/mocap/ExternalMocap.h"

#include <cstdint>
#include <string>

namespace bdfr::runtime {

class UdpFrameSender {
public:
    UdpFrameSender();
    ~UdpFrameSender();

    UdpFrameSender(const UdpFrameSender&) = delete;
    UdpFrameSender& operator=(const UdpFrameSender&) = delete;

    bool open(std::string* error = nullptr);
    void close();
    bool isOpen() const noexcept;

    bool sendTo(const std::string& host,
                std::uint16_t port,
                const mocap::MocapPacket& packet,
                std::string* error = nullptr);

private:
    struct Impl;
    Impl* impl_;
};

class UdpFrameReceiver {
public:
    UdpFrameReceiver();
    ~UdpFrameReceiver();

    UdpFrameReceiver(const UdpFrameReceiver&) = delete;
    UdpFrameReceiver& operator=(const UdpFrameReceiver&) = delete;

    bool open(std::uint16_t port,
              const std::string& bindAddress = "0.0.0.0",
              std::string* error = nullptr);
    void close();
    bool isOpen() const noexcept;
    std::uint16_t localPort() const noexcept;

    bool receive(mocap::MocapPacket& packet,
                 int timeoutMilliseconds,
                 std::string* error = nullptr);

private:
    struct Impl;
    Impl* impl_;
};

} // namespace bdfr::runtime
