#include <cstring>
#include <utility>

#include "macros/assert.hpp"
#include "payloader.hpp"

auto Payloader::scan_to_payload(const LaserScan& scan, const uint16_t mtu) -> std::vector<std::vector<std::byte>> {
    const auto total_bytes           = scan.points.size() * sizeof(Point);
    const auto data_bytes_per_packet = mtu - sizeof(Header);
    const auto num_points_per_packet = data_bytes_per_packet / sizeof(Point);
    const auto num_packets           = (total_bytes + data_bytes_per_packet - 1) / data_bytes_per_packet;
    if(num_packets > std::numeric_limits<decltype(Header::serial_num)>::max()) {
        line_warn("too many points ", scan.points.size());
        return {};
    }

    auto index = size_t(0);
    auto ret   = std::vector<std::vector<std::byte>>();
    while(index < scan.points.size()) {
        const auto index_end          = std::min(index + num_points_per_packet, scan.points.size());
        const auto num_points_to_send = index_end - index;

        auto bytes                         = std::vector<std::byte>(sizeof(Header) + sizeof(Point) * num_points_to_send);
        *std::bit_cast<Header*>(&bytes[0]) = Header{
            .frame      = frame,
            .serial_num = uint8_t(ret.size()),
            .serial_end = uint8_t(num_packets),
            .points_len = uint8_t(num_points_to_send),
        };
        for(auto i = 0u; i < num_points_to_send; i += 1) {
            std::bit_cast<Point*>(&bytes[sizeof(Header)])[i] = Point{
                .angle    = scan.points[index + i].angle,
                .distance = scan.points[index + i].range,
            };
        }
        ret.emplace_back(std::move(bytes));
        index = index_end;
    }
    frame += 1;
    return ret;
}

auto Depayloader::payload_to_points(const std::span<const std::byte> packet) -> std::optional<std::vector<Point>> {
    ensure(packet.size() > sizeof(Header), "packet too short ", packet.size());
    const auto& header = *std::bit_cast<Header*>(packet.data());
    auto        ret    = std::vector<Point>();
    if(header.frame < frame) {
        bail("late frame ", int(frame), " vs ", int(header.frame));
    } else if(header.frame == frame) {
        ensure(!received_serials[header.serial_num], "duplicated packet arrived serial=", int(header.serial_num));
    } else { // header.frame > frame
        if(!buffer.empty()) {
            line_warn("newer frame arrived, discarding last frame ", int(frame), " vs ", int(header.frame));
            ret = std::exchange(buffer, {});
        }
        frame                     = header.frame;
        received_serials          = {};
        received_serials_popcount = 0;
    }

    const auto num_points = (packet.size() - sizeof(Header)) / sizeof(Point);
    const auto points     = std::bit_cast<Point*>(&packet[sizeof(Header)]);
    const auto prev_size  = buffer.size();
    buffer.resize(prev_size + num_points);
    std::memcpy(&buffer[prev_size], points, sizeof(Point) * num_points);

    received_serials[header.serial_num] = true;
    received_serials_popcount += 1;
    if(received_serials_popcount == header.serial_end) {
        ret = std::exchange(buffer, {});
        frame += 1;
        received_serials          = {};
        received_serials_popcount = 0;
    }
    if(!ret.empty()) {
        return ret;
    } else {
        return std::nullopt;
    }
}
