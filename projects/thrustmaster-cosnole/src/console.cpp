#include <array>

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

#include "caps.hpp"
#include "input-decoder.hpp"
#include "macros/unwrap.hpp"
#include "state-encoder.hpp"
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
    auto addr_str    = "127.0.0.1";
    auto port        = uint16_t(8080);
    auto device_path = "/dev/input/by-id/usb-Thrustmaster_Thrustmaster_T300RS_Racing_wheel-event-joystick";
    {
        auto parser = args::Parser<uint16_t>();
        auto help   = false;
        parser.kwarg(&addr_str, {"-a", "-addr"}, "ADDRESS", "destination ipv4 address", {.state = args::State::DefaultValue});
        parser.kwarg(&port, {"-p", "-port"}, "PORT", "destination port number", {.state = args::State::DefaultValue});
        parser.kwarg(&port, {"-d", "-device"}, "PATH", "controller device file path", {.state = args::State::DefaultValue});
        parser.kwflag(&help, {"-h", "--help"}, "print this help message", {.no_error_check = true});
        if(!parser.parse(argc, argv) || help) {
            print("usage: transmitter ", parser.get_help());
            return 0;
        }
    }
    unwrap(addr, parse_addr(addr_str));

    const auto dev = open(device_path, O_RDONLY | O_NONBLOCK);
    ensure(dev >= 0, "failed to open ", device_path);

    unwrap(caps, DeviceCaps::from_device(dev));
    caps.debug_print();

    const auto profile = InputProfile{
        .steering_axis   = 0,
        .accel_axis      = 5,
        .brake_axis      = 1,
        .accel_threshold = 0.15,
        .brake_threshold = 0.15,
        .invert_accel    = true,
        .invert_brake    = true,
    };

    auto event = input_event();
    auto state = ControllerState();
    unwrap_mut(decoder, InputDecoder::create(caps, profile));
    unwrap(sock, udp::create(0, 0));

    auto pollfds = std::array{pollfd{.fd = dev, .events = POLLIN}};
loop:
    ensure(poll(pollfds.data(), pollfds.size(), -1) > 0 || errno == EINTR);
    if(errno == EINTR) {
        goto loop;
    }
    ensure(pollfds[0].revents & POLLIN);
    auto updated = false;
    while(true) {
        const auto ret = read(dev, &event, sizeof(event));
        if(ret < 0) {
            if(errno == EAGAIN) {
                break;
            } else {
                bail("read() failed: ", strerror(errno));
            }
        }
        // printf("[%ld.%ld] %d %d %d\n", event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);

        if(!decoder.decode_input(event, state)) {
            continue;
        }
        updated = true;
        print("steering ", state.steering, " accel ", state.accel, " brake ", state.brake);
    }

    if(updated) {
        auto packet = encode_state(state);
        for(const auto wheel : {packet.left, packet.right}) {
            print("mode ", int(wheel.mode), " value ", wheel.value);
        }
        if(!udp::send_to(sock.as_handle(), addr, port, &packet, sizeof(packet))) {
            line_warn("failed to send packet: ", strerror(errno));
        }
    }

    goto loop;
    return 0;
}
