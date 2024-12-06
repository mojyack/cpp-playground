#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include "macros/assert.hpp"
#include "util/fd.hpp"

namespace udp {
auto create(const uint32_t addr, const uint16_t port) -> std::optional<FileDescriptor> {
    auto sock = FileDescriptor(socket(AF_INET, SOCK_DGRAM, 0));
    ensure(sock.as_handle() >= 0);

    auto sockaddr            = sockaddr_in();
    sockaddr.sin_family      = AF_INET;
    sockaddr.sin_port        = htons(port);
    sockaddr.sin_addr.s_addr = htonl(addr);

    ensure(bind(sock.as_handle(), (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == 0, strerror(errno));

    return sock;
}

auto send_to(const int sock, const uint32_t addr, const uint16_t port, const void* const data, const size_t size) -> bool {
    auto sockaddr            = sockaddr_in();
    sockaddr.sin_family      = AF_INET;
    sockaddr.sin_port        = htons(port);
    sockaddr.sin_addr.s_addr = htonl(addr);

    return sendto(sock, data, size, 0, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == ssize_t(size);
}
} // namespace udp
