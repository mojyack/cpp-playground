#include <CYdLidar.h>

#include "lidar.hpp"
#include "macros/unwrap.hpp"
#include "payloader.hpp"
#include "udp.hpp"
#include "util/argument-parser.hpp"

namespace {
auto parse_addr(const char* const addr) -> std::optional<uint32_t> {
    auto a = uint8_t(), b = uint8_t(), c = uint8_t(), d = uint8_t();
    ensure(sscanf(addr, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) == 4, "invalid ipv4 address");
    return uint32_t(a << 24 | b << 16 | c << 8 | d);
}
} // namespace

auto main(const int argc, const char* const* argv) -> int {
    auto addr_str = "127.0.0.1";
    auto port     = uint16_t(8081);
    {
        auto parser = args::Parser<uint16_t>();
        auto help   = false;
        parser.kwarg(&addr_str, {"-a", "-addr"}, "ADDRESS", "destination ipv4 address", {.state = args::State::DefaultValue});
        parser.kwarg(&port, {"-p", "-port"}, "PORT", "destination port number", {.state = args::State::DefaultValue});
        parser.kwflag(&help, {"-h", "--help"}, "print this help message", {.no_error_check = true});
        if(!parser.parse(argc, argv) || help) {
            print("usage: transmitter ", parser.get_help());
            return 0;
        }
    }
    unwrap(addr, parse_addr(addr_str));

    unwrap(sock, udp::create(0, 0));

    auto laser = CYdLidar();
    auto scan  = LaserScan();
    ensure(init_lidar(laser));

    auto payloader = Payloader();

loop:
    ensure(ydlidar::os_isOk());
    ensure(laser.doProcessSimple(scan));
    print("received ", scan.points.size(), " points, ", scan.scanFreq, " Hz");

    for(auto& packet : payloader.scan_to_payload(scan, 1400)) {
        if(!udp::send_to(sock.as_handle(), addr, port, packet.data(), packet.size())) {
            line_warn("failed to send packet: ", strerror(errno));
        }
    }

    goto loop;
}
