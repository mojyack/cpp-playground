#pragma once
#include <cstdint>
#include <optional>

#include "util/fd.hpp"

namespace udp {
inline auto to_inet_addr(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d) -> uint32_t {
    return a << 24 | b << 16 | c << 8 | d;
}

auto create(uint32_t addr, uint16_t port) -> std::optional<FileDescriptor>;
auto send_to(int sock, uint32_t addr, uint16_t port, const void* data, size_t size) -> bool;
} // namespace udp
