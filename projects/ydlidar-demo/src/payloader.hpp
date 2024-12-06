#pragma once
#include <array>
#include <span>

#include <CYdLidar.h>

#include "protocol.hpp"

struct Payloader {
    uint8_t frame = 0;

    auto scan_to_payload(const LaserScan& scan, uint16_t mtu) -> std::vector<std::vector<std::byte>>;
};

struct Depayloader {
    uint8_t                frame                     = 0;
    uint8_t                received_serials_popcount = 0;
    std::vector<Point>     buffer;
    std::array<bool, 0xFF> received_serials;

    auto payload_to_points(std::span<const std::byte> packet) -> std::optional<std::vector<Point>>;
};
