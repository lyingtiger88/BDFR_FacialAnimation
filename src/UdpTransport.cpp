#include "bdfr/runtime/UdpTransport.h"

#include "bdfr/runtime/FramePacketCodec.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketHandle = SOCKET;
constexpr SocketHandle InvalidSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
using SocketHandle = int;
constexpr SocketHandle InvalidSocket = -1;
#endif

namespace bdfr::runtime {

namespace {

#ifdef _WIN32
class WinsockRuntime {
public:
    WinsockRuntime() {
        WSADATA data{};
        ok_ = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }

    ~WinsockRuntime() {
        if (ok_) WSACleanup();
    }

    bool ok() const noexcept { return ok_; }

private:
    bool ok_ = false;
};

WinsockRuntime& winsock() {
    static WinsockRuntime instance;
    return instance;
}
#endif

bool initializeSockets(std::string* error) {
#ifdef _WIN32
    if (!winsock().ok()) {
        if (error) *error = "WSAStartup failed";
        return false;
    }
#else
    (void)error;
#endif
    return true;
}

void closeSocket(SocketHandle handle) {
    if (handle == InvalidSocket) return;
#ifdef _WIN32
    closesocket(handle);
#else
    ::close(handle);
#endif
}

std::string lastSocketError() {
#ifdef _WIN32
    return "socket error " + std::to_string(WSAGetLastError());
#else
    return std::strerror(errno);
#endif
}

bool resolveIpv4(const std::string& host,
                 std::uint16_t port,
                 sockaddr_in& address,
                 std::string* error) {
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) == 1) {
        return true;
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    addrinfo* result = nullptr;
    const int rc = getaddrinfo(host.c_str(), nullptr, &hints, &result);
    if (rc != 0 || result == nullptr) {
        if (error) *error = "failed to resolve UDP host: " + host;
        if (result) freeaddrinfo(result);
        return false;
    }

    address.sin_addr =
        reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr;
    freeaddrinfo(result);
    return true;
}

} // namespace

struct UdpFrameSender::Impl {
    SocketHandle socket = InvalidSocket;
};

UdpFrameSender::UdpFrameSender()
    : impl_(new Impl) {}

UdpFrameSender::~UdpFrameSender() {
    close();
    delete impl_;
}

bool UdpFrameSender::open(std::string* error) {
    if (isOpen()) return true;
    if (!initializeSockets(error)) return false;

    impl_->socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (impl_->socket == InvalidSocket) {
        if (error) *error = lastSocketError();
        return false;
    }
    return true;
}

void UdpFrameSender::close() {
    closeSocket(impl_->socket);
    impl_->socket = InvalidSocket;
}

bool UdpFrameSender::isOpen() const noexcept {
    return impl_->socket != InvalidSocket;
}

bool UdpFrameSender::sendTo(const std::string& host,
                            std::uint16_t port,
                            const mocap::MocapPacket& packet,
                            std::string* error) {
    if (!isOpen() && !open(error)) return false;

    sockaddr_in address{};
    if (!resolveIpv4(host, port, address, error)) return false;

    const auto bytes = FramePacketCodec::encode(packet);

#ifdef _WIN32
    const int sent = ::sendto(
        impl_->socket,
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<int>(bytes.size()),
        0,
        reinterpret_cast<const sockaddr*>(&address),
        static_cast<int>(sizeof(address)));
#else
    const ssize_t sent = ::sendto(
        impl_->socket,
        bytes.data(),
        bytes.size(),
        0,
        reinterpret_cast<const sockaddr*>(&address),
        sizeof(address));
#endif

    if (sent < 0 || static_cast<std::size_t>(sent) != bytes.size()) {
        if (error) *error = lastSocketError();
        return false;
    }

    return true;
}

struct UdpFrameReceiver::Impl {
    SocketHandle socket = InvalidSocket;
    std::uint16_t port = 0;
};

UdpFrameReceiver::UdpFrameReceiver()
    : impl_(new Impl) {}

UdpFrameReceiver::~UdpFrameReceiver() {
    close();
    delete impl_;
}

bool UdpFrameReceiver::open(std::uint16_t port,
                            const std::string& bindAddress,
                            std::string* error) {
    close();
    if (!initializeSockets(error)) return false;

    impl_->socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (impl_->socket == InvalidSocket) {
        if (error) *error = lastSocketError();
        return false;
    }

    sockaddr_in address{};
    if (!resolveIpv4(bindAddress, port, address, error)) {
        close();
        return false;
    }

    if (::bind(
            impl_->socket,
            reinterpret_cast<const sockaddr*>(&address),
            static_cast<socklen_t>(sizeof(address))) != 0) {
        if (error) *error = lastSocketError();
        close();
        return false;
    }

    sockaddr_in bound{};
#ifdef _WIN32
    int length = static_cast<int>(sizeof(bound));
#else
    socklen_t length = sizeof(bound);
#endif

    if (getsockname(
            impl_->socket,
            reinterpret_cast<sockaddr*>(&bound),
            &length) != 0) {
        if (error) *error = lastSocketError();
        close();
        return false;
    }

    impl_->port = ntohs(bound.sin_port);
    return true;
}

void UdpFrameReceiver::close() {
    closeSocket(impl_->socket);
    impl_->socket = InvalidSocket;
    impl_->port = 0;
}

bool UdpFrameReceiver::isOpen() const noexcept {
    return impl_->socket != InvalidSocket;
}

std::uint16_t UdpFrameReceiver::localPort() const noexcept {
    return impl_->port;
}

bool UdpFrameReceiver::receive(mocap::MocapPacket& packet,
                               int timeoutMilliseconds,
                               std::string* error) {
    if (!isOpen()) {
        if (error) *error = "UDP receiver is not open";
        return false;
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(impl_->socket, &readSet);

    const int timeoutMs = std::max(0, timeoutMilliseconds);
    timeval timeout{};
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

#ifdef _WIN32
    const int ready = select(0, &readSet, nullptr, nullptr, &timeout);
#else
    const int ready = select(impl_->socket + 1, &readSet, nullptr, nullptr, &timeout);
#endif

    if (ready <= 0) {
        if (ready < 0 && error) *error = lastSocketError();
        return false;
    }

    std::array<std::uint8_t, 65507> buffer{};

#ifdef _WIN32
    const int received = recvfrom(
        impl_->socket,
        reinterpret_cast<char*>(buffer.data()),
        static_cast<int>(buffer.size()),
        0,
        nullptr,
        nullptr);
#else
    const ssize_t received = recvfrom(
        impl_->socket,
        buffer.data(),
        buffer.size(),
        0,
        nullptr,
        nullptr);
#endif

    if (received <= 0) {
        if (error) *error = lastSocketError();
        return false;
    }

    std::vector<std::uint8_t> bytes(
        buffer.begin(),
        buffer.begin() + static_cast<std::ptrdiff_t>(received));

    if (!FramePacketCodec::decode(bytes, packet)) {
        if (error) *error = "invalid BDFR UDP packet";
        return false;
    }

    return true;
}

} // namespace bdfr::runtime
